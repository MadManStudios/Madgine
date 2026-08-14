#pragma once

#include "Generic/containers/generator.h"
#include "Generic/execution/signal.h"
#include "Generic/execution/intervalclock.h"
#include "Generic/projections.h"

#include "Platform/input/inputevents.h"

#include "Meta/math/atlas2.h"

#include "Madgine/behavior/named.h"
#include "Madgine/debug/debuggablelifetime.h"
#include "Madgine/imageloader/imageloader.h"
#include "Madgine/window/mainwindowcomponent.h"
#include "Madgine/window/mainwindowcomponentcollector.h"

#include "events.h"
#include "layoutwidget.h"
#include "widgetloader.h"

namespace Engine {
namespace Widgets {

    struct MADGINE_WIDGETS_EXPORT WidgetManager : Core::MainWindowComponent<WidgetManager> {

        SERIALIZABLEUNIT(WidgetManager)

        WidgetManager(Core::MainWindow &window);
        WidgetManager(const WidgetManager &sharedInstance);
        ~WidgetManager();

        void swapCurrentRoot(std::string_view name);
        void swapCurrentRoot(WidgetBase *newRoot);
        void openModalWidget(WidgetBase *widget);
        void closeModalWidget(WidgetBase *widget);
        void openWidget(WidgetBase *widget);
        void closeWidget(WidgetBase *widget);
        void openOverlay(WidgetBase *widget);
        void closeOverlay(WidgetBase *widget);

        void openLayout(std::string_view name);
        void closeLayout(std::string_view name);

        void createLayout(std::string_view name);
        LayoutWidget *getLayoutWidget(std::string_view name);
        std::list<LayoutWidget> &layoutWidgets();

        bool isHovered(WidgetBase *w);
        WidgetBase *hoveredWidget();
        WidgetBase *focusedWidget();
        WidgetBase *pointerEventTargetWidget();

        WidgetBase *getWidget(std::string_view name);

        void registerWidget(WidgetBase *w);
        void unregisterWidget(WidgetBase *w);

        virtual Threading::Task<bool> init() override;
        virtual Threading::Task<void> finalize() override;

        void startLifetime() override;
        void endLifetime();

        WidgetBase *currentRoot();

        void clear();

        void destroyTopLevel(WidgetBase *w);
        WidgetBase *createTopLevel();

        decltype(auto) widgets()
        {
            return mTopLevelWidgets | std::views::transform(projectionUniquePtrToPtr);
        }

        bool onWindowEvent(const Platform::Window::WindowEvent &arg) override;
        bool injectPointerPress(const Platform::Input::PointerPressEvent &arg);
        bool injectPointerRelease(const Platform::Input::PointerReleaseEvent &arg);
        bool injectPointerMove(const Platform::Input::PointerMoveEvent &arg);
        bool injectAxisEvent(const Platform::Input::AxisEvent &arg);
        bool injectKeyPress(const Platform::Input::KeyPressEvent &arg);
        bool injectKeyRelease(const Platform::Input::KeyReleaseEvent &arg);

        void onResize(const Math::Rect2i &space) override;
        void setup(Render::RenderTarget *target) override;
        void render(Render::RenderTarget *target, size_t iteration) override;

        void render(Render::RenderTarget *target, const WidgetsRenderData &renderData, const Math::Vector2i &size);

        Resources::ImageLoader::Resource *getImage(std::string_view name);

        const Math::Atlas2::Entry *lookUpImage(Resources::ImageLoader::Resource *image);
        const Math::Atlas2::Entry *lookUpImage(std::string_view name);

        bool dragging(const WidgetBase *widget);
        void abortDrag(WidgetBase *widget);

        Debug::DebuggableLifetime<Behavior::get_named_d> &lifetime();

        Execution::IntervalClock<> &clock();

        using RenderPass::addDependency;
        using RenderPass::removeDependency;

        static Serialize::StreamResult scanWidget(const Serialize::SerializeTable *&out, Serialize::FormattedSerializeStream &in);

    protected:
        WidgetBase *getHoveredWidget(const Math::Vector2 &pos, WidgetBase *current);
        WidgetBase *getHoveredWidgetUp(const Math::Vector2 &pos, WidgetBase *current);
        WidgetBase *getHoveredWidgetDown(const Math::Vector2 &pos, WidgetBase *current);

        void resetPointerState();

        Serialize::StreamResult readWidget(Serialize::FormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget, WidgetBase *parent);
        Serialize::StreamResult readWidgetStub(Serialize::FormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget);
        const char *writeWidget(Serialize::FormattedSerializeStream &out, const std::unique_ptr<WidgetBase> &widget) const;

        friend struct WidgetBase;

        void onActivate(Serialize::CallbackTiming timing, bool active);

    private:
        std::vector<WidgetBase *> mWidgets;

        std::vector<std::unique_ptr<WidgetBase>> mTopLevelWidgets;

        std::list<LayoutWidget> mWidgetsLayout;

        WidgetBase *mHoveredWidget = nullptr;
        WidgetBase *mFocusedWidget = nullptr;

        WidgetBase *mPointerEventTargetWidget = nullptr;

        WidgetBase *mCurrentRoot = nullptr;

        std::vector<WidgetBase *> mModalWidgetList;
        std::vector<WidgetBase *> mOverlays;

        DEBUGGABLE_LIFETIME(mLifetime, Behavior::get_named_d);

        Execution::IntervalClock<> mFrameClock = std::chrono::steady_clock::now();

        struct WidgetManagerData;
        std::shared_ptr<WidgetManagerData> mData;

        // Dragging
        DragBeginEvent mDragStartEvent { { 0, 0 }, { 0, 0 }, Platform::Input::MouseButton::NO_BUTTON };
        bool mDragging = false;
        bool mDraggingAborted = false;
        std::chrono::steady_clock::time_point mDragStartTime;

        Math::Vector2 mShadowOffset = { 0, 0 };
    };

}
}