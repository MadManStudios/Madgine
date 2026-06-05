#pragma once

#include "Modules/threading/madgineobject.h"
#include "Modules/threading/taskqueue.h"
#include "Modules/uniquecomponent/uniquecomponentcontainer.h"
#include "Modules/uniquecomponent/uniquecomponentselector.h"

#include "Madgine/debug/debuggablelifetime.h"
#include "Madgine/render/rendercontextcollector.h"

#include "layoutloader.h"
#include "mainwindowcomponentcollector.h"

namespace Engine {
namespace Core {

    struct MADGINE_CLIENT_EXPORT MainWindowComponentComparator {

        bool operator()(const std::unique_ptr<MainWindowComponentBase> &first, const std::unique_ptr<MainWindowComponentBase> &second) const;

        struct traits {
            using type = int;
            using item_type = std::unique_ptr<MainWindowComponentBase>;

            static int to_cmp_type(const item_type &value);
        };
    };

    /**
     * @brief The MainWindow manages multiple concepts relevant for client applications.
     *
     * The responsibilities of the class include:
     *  - creation of an OSWindow
     *  - creation of a RenderContext and a RenderTarget for the OSWindow
     *  - creation of a TaskQueue for the render thread
     *  - creation & management of all MainWindowComponents
     *
     * The MainWindowComponents are ordered by priority. The order is relevant for input-event
     * propagation and render order.
     *
     */
    struct MADGINE_CLIENT_EXPORT MainWindow : Threading::MadgineObject<MainWindow> {
        SERIALIZABLEUNIT(MainWindow)

        MainWindow(Application &app, const Platform::Window::WindowSettings &settings);
        ~MainWindow();

        void saveLayout(const Platform::Filesystem::Path &path);
        Threading::Task<bool> loadLayout(LayoutLoader::Resource *res);

        /**
         * @name MadgineObject interface
         */
        ///@{
        Threading::Task<bool> init();
        Threading::Task<void> finalize();
        ///@}

        Threading::Task<void> renderLoop();

        void startLifetime();
        void endLifetime();

        Debug::DebuggableLifetime<> &lifetime();

        /**
         * @name Components
         */
        ///@{
        auto &components()
        {
            return mComponents;
        }

        template <typename T>
        T &getWindowComponent()
        {
            return static_cast<T &>(getWindowComponent(Plugins::component_index<T>()));
        }

        MainWindowComponentBase &getWindowComponent(size_t i);

        Math::Rect2i getScreenSpace();
        void applyClientSpaceResize(MainWindowComponentBase *component = nullptr);
        ///@}

        ToolWindow *createToolWindow(const Platform::Window::WindowSettings &settings);
        void destroyToolWindow(ToolWindow *w);

        Application &app() const;

        Platform::Window::OSWindow *osWindow() const;

        Render::RenderContext *getRenderer();
        Render::RenderTarget *getRenderWindow();

        Threading::TaskQueue *taskQueue();
        void shutdown();

        bool onWindowEvent(const Platform::Window::WindowEvent &event, MainWindowComponentBase *component = nullptr);

        // TESTING
        static void sTestScreens(size_t n);

    protected:
        void storeWindowData();

        void onActivate(Serialize::CallbackTiming timing, bool active);

    private:
        Application &mApp;

        const Platform::Window::WindowSettings &mSettings;

        Threading::TaskQueue mTaskQueue;

        DEBUGGABLE_LIFETIME(mLifetime);

        MainWindowComponentContainer<std::set<Placeholder<0>, MainWindowComponentComparator>> mComponents;

        std::list<ToolWindow> mToolWindows;

        Platform::Window::OSWindow *mOsWindow = nullptr;
        Render::RenderContextSelector mRenderContext;
        std::unique_ptr<Render::RenderTarget> mRenderWindow;

        bool mSoftwareKeyboardRequested = false;
    };

}
}
