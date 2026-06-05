#include "../clientlib.h"
#include "Madgine/serialize/filesystem/filesystemlib.h"

#include "mainwindow.h"

#include "Generic/execution/execution.h"
#include "Generic/projections.h"

#include "Platform/filesystem/fsapi.h"
#include "Platform/window/windowapi.h"
#include "Platform/window/windowsettings.h"

#include "Meta/serialize/configs/controlled.h"
#include "Meta/serialize/formats.h"

#include "Modules/debug/profiler/profile.h"
#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/resources/resourcemanager.h"
#include "Madgine/serialize/filesystem/filemanager.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "layoutloader.h"
#include "mainwindowcomponent.h"
#include "toolwindow.h"

namespace Engine {
namespace Core {
    static bool filterComponent(const std::unique_ptr<MainWindowComponentBase> &comp)
    {
        return comp->includeInLayout();
    }
    static Serialize::StreamResult staticTypeResolve(const Serialize::SerializeTable *&out, std::string_view key)
    {
        out = MainWindowComponentRegistry::get(MainWindowComponentRegistry::sComponentsByName().at(key)).mType;
        return {};
    }
}
}

METATABLE_BEGIN(Engine::Core::MainWindow)
    READONLY_PROPERTY(Components, components)
METATABLE_END(Engine::Core::MainWindow)

SERIALIZETABLE_BEGIN(Engine::Core::MainWindow)
    FIELD(mComponents,
        Serialize::ControlledConfig<
            KeyCompare<std::unique_ptr<Engine::Core::MainWindowComponentBase>>,
            Engine::Core::staticTypeResolve>,
        Serialize::CustomFilter<Engine::Core::filterComponent>)
SERIALIZETABLE_END(Engine::Core::MainWindow)

SERIALIZETABLE_BEGIN(Engine::Platform::Window::WindowData)
    FIELD(mPosition)
    FIELD(mSize)
    FIELD(mMaximized)
SERIALIZETABLE_END(Engine::Platform::Window::WindowData)

SERIALIZETABLE_BEGIN(Engine::Platform::PlatformVector)
    FIELD(x)
    FIELD(y)
SERIALIZETABLE_END(Engine::Platform::PlatformVector)

namespace Engine {
namespace Core {

    static std::queue<Platform::Window::WindowData> sTestPositions;
    static std::mutex sTestPositionMutex;

    bool MainWindowComponentComparator::operator()(const std::unique_ptr<MainWindowComponentBase> &first, const std::unique_ptr<MainWindowComponentBase> &second) const
    {
        return first->mPriority < second->mPriority;
    }

    int MainWindowComponentComparator::traits::to_cmp_type(const item_type &value)
    {
        return value->mPriority;
    }

    /**
     * @brief Creates a MainWindow and sets up its TaskQueue
     * @param settings settings for the creation of OSWindow
     *
     * The settings are stored by reference. Instantiates all MainWindowComponents.
     * Initialization/Deinitialization-tasks of the MadgineObject are registered as
     * setup steps in the TaskQueue. render() is registered as repeated task to the
     * TaskQueue.
     */
    MainWindow::MainWindow(Application &app, const Platform::Window::WindowSettings &settings)
        : mApp(app)
        , mSettings(settings)
        , mTaskQueue("FrameLoop", true)
        , mComponents(*this)
        , mRenderContext(&mTaskQueue)
    {
        mTaskQueue.addSetupSteps(
            [this]() { return callInit(); },
            [this]() { return callFinalize(); });
    }

    /**
     * @brief default destructor
     */
    MainWindow::~MainWindow() = default;

    void MainWindow::saveLayout(const Platform::Filesystem::Path &path)
    {
        Serialize::FileManager mgr { "Layout" };
        Serialize::FormattedSerializeStream file = mgr.openWrite(path, Serialize::Formats::xml);

        if (file) {
            Serialize::write(file, *this, "Layout");
        } else {
            LOG_ERROR("Failed to open \"" << path << "\" for write!");
        }
    }

    Threading::Task<bool> MainWindow::loadLayout(LayoutLoader::Resource *res)
    {
        return res->loadTask(*this);
    }

    /**
     * @brief
     * @return
     */
    Threading::Task<bool> MainWindow::init()
    {
        Platform::Window::WindowSettings settings = mSettings;

        if (!sTestPositions.empty()) {
            std::unique_lock lock { sTestPositionMutex };
            settings.mData = sTestPositions.front();
            sTestPositions.pop();
        } else if (settings.mRestoreGeometry) {
            Serialize::FileManager mgr { "MainWindow-Geometry" };

            Platform::Filesystem::Path path = Platform::Filesystem::appDataPath() / "mainwindow.ini";

            if (Serialize::FormattedSerializeStream in = mgr.openRead(path, Serialize::Formats::ini)) {
                Serialize::StreamResult result = read(in, settings.mData, nullptr);
                if (result.mState != Serialize::StreamState::OK) {
                    LOG_ERROR("Error loading MainWindow-Geometry from " << path << ": \n"
                                                                        << result);
                }
            }
        }

        mOsWindow = Platform::Window::sCreateWindow(settings);
        mRenderWindow = mRenderContext->createRenderWindow(mOsWindow);

        for (const std::unique_ptr<MainWindowComponentBase> &comp : components()) {
            [[maybe_unused]] bool result = co_await comp->callInit();
            assert(result);
        }

        applyClientSpaceResize();

#ifdef MADGINE_MAINWINDOW_LAYOUT
        std::string_view name = StringUtil::tokenize<2>(STRINGIFY2(MADGINE_MAINWINDOW_LAYOUT), ':')[1];
        LayoutLoader::Resource *res = LayoutLoader::get(name);
        if (!res) {
            LOG_ERROR("Unable to load layout '" << name << "'!");
            co_return false;
        }

        if (!co_await loadLayout(res))
            co_return false;
#endif

        // applyClientSpaceResize();

        mTaskQueue.queueTask(renderLoop());

        startLifetime();

        co_return true;
    }

    /**
     * @brief
     * @return
     */
    Threading::Task<void> MainWindow::finalize()
    {
        for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
            co_await comp->callFinalize();
        }

        co_await mRenderContext->unloadAllResources();

        mRenderWindow.reset();

        if (mOsWindow) {
            storeWindowData();
            mOsWindow->destroy();
            mOsWindow = nullptr;
        }

        co_return;
    }

    Threading::Task<void> MainWindow::renderLoop()
    {
        std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
        while (mTaskQueue.running()) {
            bool wantSoftwareKeyboard = false;
            for (const std::unique_ptr<MainWindowComponentBase> &comp : components()) {
                wantSoftwareKeyboard |= comp->wantsSoftwareKeyboard();
            }
            if (mSoftwareKeyboardRequested != wantSoftwareKeyboard) {
                mSoftwareKeyboardRequested = wantSoftwareKeyboard;
                if (wantSoftwareKeyboard)
                    mOsWindow->requestSoftwareKeyboard();
                else
                    mOsWindow->releaseSoftwareKeyboard();
            }
            co_await mRenderContext->render();
            {
                PROFILE_NAMED("Window Update");
                while (std::optional<Platform::Window::WindowEvent> event = mOsWindow ? mOsWindow->update() : std::nullopt) {
                    onWindowEvent(*event);
                }
            }
            for (ToolWindow &window : mToolWindows)
                while (std::optional<Platform::Window::WindowEvent> event = window.osWindow()->update()) {
                    window.onWindowEvent(*event);
                }
            now += (1000000us / 1200);
            co_await 0ms;
        }
    }

    void MainWindow::startLifetime()
    {
        mTaskQueue.queue([this]()->Threading::ImmediateTask<void> {
            co_await mLifetime;
        });
        for (const std::unique_ptr<MainWindowComponentBase> &comp : components()) {
            comp->startLifetime();
        }
    }

    void MainWindow::endLifetime()
    {
        mLifetime.end();
    }

    Debug::DebuggableLifetime<> &MainWindow::lifetime()
    {
        return mLifetime;
    }

    Application &MainWindow::app() const
    {
        return mApp;
    }

    /**
     * @brief
     * @param i
     * @return
     */
    MainWindowComponentBase &MainWindow::getWindowComponent(size_t i)
    {
        return mComponents.get(i);
    }

    /**
     * @brief
     * @param settings
     * @return
     */
    ToolWindow *MainWindow::createToolWindow(const Platform::Window::WindowSettings &settings)
    {
        return &mToolWindows.emplace_back(*this, settings);
    }

    /**
     * @brief
     * @param w
     */
    void MainWindow::destroyToolWindow(ToolWindow *w)
    {
        auto it = std::ranges::find(mToolWindows, w, projectionAddressOf);
        assert(it != mToolWindows.end());
        mToolWindows.erase(it);
    }

    /**
     * @brief Returns a pointer to the OSWindow
     * @return the OSWindow
     */
    Platform::Window::OSWindow *MainWindow::osWindow() const
    {
        return mOsWindow;
    }

    /**
     * @brief Returns a pointer to the RenderContext
     * @return the RenderContext
     */
    Render::RenderContext *MainWindow::getRenderer()
    {
        return mRenderContext;
    }

    /**
     * @brief Returns the pointer to the RenderWindow
     * @return the RenderWindow
     */
    Render::RenderTarget *MainWindow::getRenderWindow()
    {
        return mRenderWindow.get();
    }

    /**
     * @brief Returns a pointer to the TaskQueue
     * @return the TaskQueue
     */
    Threading::TaskQueue *MainWindow::taskQueue()
    {
        return &mTaskQueue;
    }

    /**
     * @brief
     */
    void MainWindow::shutdown()
    {
        mTaskQueue.stop();
    }

    /**
     * @brief
     * @return
     */
    Math::Rect2i MainWindow::getScreenSpace()
    {
        if (!mOsWindow)
            return { { 0, 0 }, { 0, 0 } };
        Platform::PlatformVector pos = mOsWindow->renderPos();
        Platform::PlatformVector size = mOsWindow->renderSize();
        return {
            { pos.x, pos.y }, { size.x, size.y }
        };
    }

    /**
     * @brief
     * @param component
     */
    void MainWindow::applyClientSpaceResize(MainWindowComponentBase *component)
    {
        if (!mOsWindow)
            return;

        Math::Rect2i space;
        if (!component) {
            Platform::PlatformVector size = mOsWindow->renderSize();
            space = {
                { 0, 0 }, { size.x, size.y }
            };
        } else
            space = component->getChildClientSpace();

        for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
            if (component) {
                if (component == comp.get()) {
                    component = nullptr;
                }
            } else {
                comp->onResize(space);
                space = comp->getChildClientSpace();
            }
        }
    }

    bool MainWindow::onWindowEvent(const Platform::Window::WindowEvent &event, MainWindowComponentBase *component)
    {
        return std::visit(overloaded {
                              [this](const Platform::Window::ResizeEvent &event) {
                                  mRenderWindow->resize({ event.mSize.x, event.mSize.y });
                                  applyClientSpaceResize();
                                  return true;
                              },
                              [&, this](const Platform::Window::CloseEvent &event) {
                                  for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
                                      if (component) {
                                          if (component == comp.get())
                                              component = nullptr;
                                      } else {
                                          if (comp->onWindowEvent(event))
                                              return true;
                                      }
                                  }
                                  mTaskQueue.stop();
                                  return true;
                              },
                              [this](const Platform::Window::RepaintEvent &event) {
                                  for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
                                      comp->onWindowEvent(event);
                                  }
                                  return false;
                              },
                              [this](const auto &event) {
                                  Platform::PlatformVector storedWindowPosition;
                                  if constexpr (requires { event.mWindowPosition; }) {
                                      storedWindowPosition = event.mWindowPosition;
                                  }
                                  for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
                                      if constexpr (requires { event.mWindowPosition; }) {
                                          event.mWindowPosition = storedWindowPosition - Platform::PlatformVector { comp->getClientSpace().mTopLeft.x, comp->getClientSpace().mTopLeft.y };
                                      }
                                      if (comp->onWindowEvent(event))
                                          return true;
                                  }
                                  return false;
                              } },
            event);
    }

    /**
     * @brief
     */
    void MainWindow::storeWindowData()
    {
        Serialize::FileManager mgr { "MainWindow-Layout" };

        if (Serialize::FormattedSerializeStream out = mgr.openWrite(Platform::Filesystem::appDataPath() / "mainwindow.ini", Serialize::Formats::ini)) {
            write(out, mOsWindow->data(), "data");
        }
    }

    void MainWindow::onActivate(Serialize::CallbackTiming timing, bool active)
    {
        if ((active && timing == Serialize::CallbackTiming::AFTER) || (!active && timing == Serialize::CallbackTiming::BEFORE))
            return;

        if (state().is_ready()) {
            if (active) {
                startLifetime();
            } else {
                endLifetime();
            }
        }
    }

    void MainWindow::sTestScreens(size_t n)
    {
        int rows = ceil(sqrt(n + 0.25f) - 0.5f);
        int cols = (n - 1) / rows + 1;

        Platform::PlatformVector monitorSize = Platform::Window::listMonitors().front().mSize;

        Platform::PlatformVector size = { monitorSize.x / cols, monitorSize.y / rows };

        std::unique_lock lock { sTestPositionMutex };

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                if (row * cols + col == n)
                    break;
                sTestPositions.emplace(Platform::Window::WindowData { { col * size.x, row * size.y }, size });
            }
        }
    }

}
}
