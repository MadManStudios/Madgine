#include "../clientlib.h"
#include "Madgine/serialize/filesystem/filesystemlib.h"

#include "mainwindow.h"
#include "mainwindowcomponent.h"
#include "toolwindow.h"

#include "Interfaces/filesystem/fsapi.h"
#include "Interfaces/window/windowapi.h"
#include "Interfaces/window/windowsettings.h"

#include "Madgine/serialize/filesystem/filemanager.h"
#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/configs/controlled.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/serialize/formats.h"

#include "../render/rendercontext.h"
#include "../render/rendertarget.h"

#include "Modules/debug/profiler/profile.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "Madgine/resources/resourcemanager.h"

#include "Generic/projections.h"

#include "layoutloader.h"

#include "Modules/threading/awaitables/awaitablesender.h"

namespace Engine {
namespace Window {
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

METATABLE_BEGIN(Engine::Window::MainWindow)
READONLY_PROPERTY(Components, components)
METATABLE_END(Engine::Window::MainWindow)

SERIALIZETABLE_BEGIN(Engine::Window::MainWindow)
FIELD(mComponents,
    Serialize::ControlledConfig<
        KeyCompare<std::unique_ptr<Engine::Window::MainWindowComponentBase>>,
        Engine::Window::staticTypeResolve>,
    Serialize::CustomFilter<Engine::Window::filterComponent>)
SERIALIZETABLE_END(Engine::Window::MainWindow)

namespace Engine {
namespace Window {

    static std::queue<WindowData> sTestPositions;
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
    MainWindow::MainWindow(const WindowSettings &settings)
        : mSettings(settings)
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

    void MainWindow::saveLayout(const Filesystem::Path &path)
    {
        Filesystem::FileManager mgr { "Layout" };
        Serialize::FormattedSerializeStream file = mgr.openWrite(path, Serialize::Formats::xml);

        if (file) {
            Serialize::write(file, *this, "Layout");
        } else {
            LOG_ERROR("Failed to open \"" << path << "\" for write!");
        }
    }

    bool MainWindow::loadLayout(std::string_view name)
    {
        LayoutLoader::Resource *res = LayoutLoader::get(name);

        if (res) {
            Serialize::SerializeManager mgr { "Layout" };
            Serialize::FormattedSerializeStream file = res->readAsFormattedStream(mgr);

            if (file) {
                Serialize::StreamResult result = Serialize::read(file, *this, nullptr, {}, Serialize::StateTransmissionFlags_ApplyMap | Serialize::StateTransmissionFlags_Activation);
                if (result.mState != Serialize::StreamState::OK) {
                    LOG_ERROR("Failed loading '" << res->path() << "' with following Error: "
                                                 << "\n"
                                                 << result);
                    return false;
                }
                return true;
            } else {
                LOG_ERROR("Failed to open " << res->path() << "!");
                return false;
            }
        } else {
            LOG_ERROR("Could not find layout " << name << "!");
            return false;
        }
    }

    /**
     * @brief 
     * @return 
    */
    Threading::Task<bool> MainWindow::init()
    {
        WindowSettings settings = mSettings;

        if (!sTestPositions.empty()) {
            std::unique_lock lock { sTestPositionMutex };
            settings.mData = sTestPositions.front();
            sTestPositions.pop();
        } else if (settings.mRestoreGeometry) {
            Filesystem::FileManager mgr { "MainWindow-Geometry" };

            Filesystem::Path path = Filesystem::appDataPath() / "mainwindow.ini";

            if (Serialize::FormattedSerializeStream in = mgr.openRead(path, Serialize::Formats::ini)) {
                Serialize::StreamResult result = read(in, settings.mData, nullptr);
                if (result.mState != Serialize::StreamState::OK) {
                    LOG_ERROR("Error loading MainWindow-Geometry from " << path << ": \n"
                                                                        << result);
                }
            }
        }

        mOsWindow = sCreateWindow(settings, this);
        mRenderWindow = mRenderContext->createRenderWindow(mOsWindow);

        for (const std::unique_ptr<MainWindowComponentBase> &comp : components()) {
            bool result = co_await comp->callInit();
            assert(result);
        }

        applyClientSpaceResize();

#ifdef MADGINE_MAINWINDOW_LAYOUT
        if (!loadLayout(STRINGIFY2(MADGINE_MAINWINDOW_LAYOUT)))
            co_return false;
#endif

        //applyClientSpaceResize();

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
        co_await mLifetime.end();

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
            co_await mRenderContext->render();
            {
                PROFILE_NAMED("Window Update");
                mOsWindow->update();
            }
            for (ToolWindow &window : mToolWindows)
                window.osWindow()->update();
            now += (1000000us / 1200);
            co_await 0ms;
        }
    }

    void MainWindow::startLifetime()
    {
        mLifetime.start();
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

    void MainWindow::addListener(MainWindowListener *listener)
    {
        mListeners.push_back(listener);
    }

    void MainWindow::removeListener(MainWindowListener *listener)
    {
        std::erase(mListeners, listener);
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
    ToolWindow *MainWindow::createToolWindow(const WindowSettings &settings)
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
    OSWindow *MainWindow::osWindow() const
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
    Rect2i MainWindow::getScreenSpace()
    {
        if (!mOsWindow)
            return { { 0, 0 }, { 0, 0 } };
        InterfacesVector pos = mOsWindow->renderPos();
        InterfacesVector size = mOsWindow->renderSize();
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

        Rect2i space;
        if (!component) {
            InterfacesVector size = mOsWindow->renderSize();
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

    /**
     * @brief 
     * @param arg 
     * @return 
    */
    bool MainWindow::injectKeyPress(const Input::KeyEventArgs &arg)
    {
        for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
            if (comp->injectKeyPress(arg))
                return true;
        }
        /* if (arg.mControlKeys.mAlt && arg.scancode == Input::Key::Return)
            mOsWindow->setFullscreen(!mOsWindow->isFullscreen());*/
        return false;
    }

    /**
     * @brief 
     * @param arg 
     * @return 
    */
    bool MainWindow::injectKeyRelease(const Input::KeyEventArgs &arg)
    {
        for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
            if (comp->injectKeyRelease(arg))
                return true;
        }
        return false;
    }

    /**
     * @brief 
     * @param arg 
     * @return 
    */
    bool MainWindow::injectPointerPress(const Input::PointerEventArgs &arg)
    {
        InterfacesVector storedWindowPosition = arg.windowPosition;
        for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
            arg.windowPosition = storedWindowPosition - InterfacesVector { comp->getClientSpace().mTopLeft.x, comp->getClientSpace().mTopLeft.y };
            if (comp->injectPointerPress(arg))
                return true;
        }

        return false;
    }

    /**
     * @brief 
     * @param arg 
     * @return 
    */
    bool MainWindow::injectPointerRelease(const Input::PointerEventArgs &arg)
    {
        InterfacesVector storedWindowPosition = arg.windowPosition;
        for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
            arg.windowPosition = storedWindowPosition - InterfacesVector { comp->getClientSpace().mTopLeft.x, comp->getClientSpace().mTopLeft.y };
            if (comp->injectPointerRelease(arg))
                return true;
        }

        return false;
    }

    /**
     * @brief 
     * @param arg 
     * @return 
    */
    bool MainWindow::injectPointerMove(const Input::PointerEventArgs &arg)
    {
        InterfacesVector storedWindowPosition = arg.windowPosition;
        for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
            arg.windowPosition = storedWindowPosition - InterfacesVector { comp->getClientSpace().mTopLeft.x, comp->getClientSpace().mTopLeft.y };
            if (comp->injectPointerMove(arg))
                return true;
        }

        return false;
    }

    /**
     * @brief 
     * @param arg 
     * @return 
    */
    bool MainWindow::injectAxisEvent(const Input::AxisEventArgs &arg)
    {
        for (const std::unique_ptr<MainWindowComponentBase> &comp : components() | std::views::reverse) {
            if (comp->injectAxisEvent(arg))
                return true;
        }

        return false;
    }

    /**
     * @brief 
    */
    void MainWindow::onClose()
    {
        storeWindowData();
        mOsWindow = nullptr;
        mTaskQueue.stop();
    }

    /**
     * @brief 
    */
    void MainWindow::onRepaint()
    {
    }

    /**
     * @brief 
     * @param size 
    */
    void MainWindow::onResize(const InterfacesVector &size)
    {
        mRenderWindow->resize({ size.x, size.y });
        applyClientSpaceResize();
    }

    /**
     * @brief 
    */
    void MainWindow::storeWindowData()
    {
        Filesystem::FileManager mgr { "MainWindow-Layout" };

        if (Serialize::FormattedSerializeStream out = mgr.openWrite(Filesystem::appDataPath() / "mainwindow.ini", Serialize::Formats::ini)) {
            write(out, mOsWindow->data(), "data");
        }
    }

    void MainWindow::onActivate(bool active)
    {
        if (active) {
            startLifetime();
        } else {
            endLifetime();
        }
        for (MainWindowListener *listener : mListeners)
            listener->onActivate(active);
    }

    void MainWindow::sTestScreens(size_t n)
    {
        int rows = ceil(sqrt(n + 0.25f) - 0.5f);
        int cols = (n - 1) / rows + 1;

        InterfacesVector monitorSize = listMonitors().front().mSize;

        InterfacesVector size = { monitorSize.x / cols, monitorSize.y / rows };

        std::unique_lock lock { sTestPositionMutex };

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                if (row * cols + col == n)
                    break;
                sTestPositions.emplace(WindowData { { col * size.x, row * size.y }, size });
            }
        }
    }

}
}
