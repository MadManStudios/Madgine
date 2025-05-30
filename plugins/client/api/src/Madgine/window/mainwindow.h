#pragma once

#include "Interfaces/window/windoweventlistener.h"

#include "Madgine/render/rendercontextcollector.h"

#include "mainwindowcomponentcollector.h"

#include "Modules/threading/madgineobject.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"
#include "Modules/uniquecomponent/uniquecomponentselector.h"

#include "Modules/threading/taskqueue.h"

#include "Madgine/debug/debuggablelifetime.h"

#include "mainwindowlistener.h"

namespace Engine {
namespace Window {

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
    struct MADGINE_CLIENT_EXPORT MainWindow : WindowEventListener,
                                              Threading::MadgineObject<MainWindow> {
        SERIALIZABLEUNIT(MainWindow)

        MainWindow(const WindowSettings &settings);
        ~MainWindow();

        void saveLayout(const Filesystem::Path &path);
        bool loadLayout(std::string_view name);

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

        void addListener(MainWindowListener *listener);
        void removeListener(MainWindowListener *listener);

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
            return static_cast<T &>(getWindowComponent(UniqueComponent::component_index<T>()));
        }

        MainWindowComponentBase &getWindowComponent(size_t i);

        Rect2i getScreenSpace();
        void applyClientSpaceResize(MainWindowComponentBase *component = nullptr);
        ///@}

        ToolWindow *createToolWindow(const WindowSettings &settings);
        void destroyToolWindow(ToolWindow *w);

        OSWindow *osWindow() const;

        Render::RenderContext *getRenderer();
        Render::RenderTarget *getRenderWindow();

        Threading::TaskQueue *taskQueue();
        void shutdown();

           /**
     * @name Input propagation
    */
        ///@{
        bool injectKeyPress(const Input::KeyEventArgs &arg) override;
        bool injectKeyRelease(const Input::KeyEventArgs &arg) override;
        bool injectPointerPress(const Input::PointerEventArgs &arg) override;
        bool injectPointerRelease(const Input::PointerEventArgs &arg) override;
        bool injectPointerMove(const Input::PointerEventArgs &arg) override;
        bool injectAxisEvent(const Input::AxisEventArgs &arg) override;
        ///@}

        //TESTING
        static void sTestScreens(size_t n);

    protected:
        void onClose() override;
        void onRepaint() override;
        void onResize(const InterfacesVector &size) override;

        void storeWindowData();

        void onActivate(bool active);

    private:
        const WindowSettings &mSettings;

        Threading::TaskQueue mTaskQueue;

        DEBUGGABLE_LIFETIME(mLifetime);

        MainWindowComponentContainer<std::set<Placeholder<0>, MainWindowComponentComparator>> mComponents;

        std::list<ToolWindow> mToolWindows;

        OSWindow *mOsWindow = nullptr;
        Render::RenderContextSelector mRenderContext;
        std::unique_ptr<Render::RenderTarget> mRenderWindow;

        std::vector<MainWindowListener *> mListeners;
    };

}
}
