#pragma once

#include "Generic/coroutines/generator.h"
#include "Generic/execution/signal.h"
#include "Generic/intervalclock.h"
#include "Generic/projections.h"

#include "Interfaces/input/inputevents.h"

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

    struct MADGINE_WIDGETS_EXPORT WidgetManager : Window::MainWindowComponent<WidgetManager> {

        SERIALIZABLEUNIT(WidgetManager)

        WidgetManager(Window::MainWindow &window);
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

        bool onWindowEvent(const Window::WindowEvent &arg) override;
        bool injectPointerPress(const Input::PointerPressEvent &arg);
        bool injectPointerRelease(const Input::PointerReleaseEvent &arg);
        bool injectPointerMove(const Input::PointerMoveEvent &arg);
        bool injectAxisEvent(const Input::AxisEvent &arg);
        bool injectKeyPress(const Input::KeyPressEvent &arg);
        bool injectKeyRelease(const Input::KeyReleaseEvent &arg);

        void onResize(const Rect2i &space) override;
        void setup(Render::RenderTarget *target) override;
        void render(Render::RenderTarget *target, size_t iteration) override;

        void render(Render::RenderTarget *target, const WidgetsRenderData &renderData, const Vector2i &size);

        Resources::ImageLoader::Resource *getImage(std::string_view name);

        const Atlas2::Entry *lookUpImage(Resources::ImageLoader::Resource *image);
        const Atlas2::Entry *lookUpImage(std::string_view name);

        bool dragging(const WidgetBase *widget);
        void abortDrag(WidgetBase *widget);

        Debug::DebuggableLifetime<Behavior::get_named_d> &lifetime();

        IntervalClock<> &clock();

        using RenderPass::addDependency;
        using RenderPass::removeDependency;

        static Serialize::StreamResult scanWidget(const Serialize::SerializeTable *&out, Serialize::CallerHierarchyFormattedSerializeStream &in);

    protected:
        WidgetBase *getHoveredWidget(const Vector2 &pos, WidgetBase *current);
        WidgetBase *getHoveredWidgetUp(const Vector2 &pos, WidgetBase *current);
        WidgetBase *getHoveredWidgetDown(const Vector2 &pos, WidgetBase *current);

        void resetPointerState();

        Serialize::StreamResult readWidget(Serialize::CallerHierarchyFormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget, WidgetBase *parent);
        Serialize::StreamResult readWidgetStub(Serialize::CallerHierarchyFormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget);
        const char *writeWidget(Serialize::CallerHierarchyFormattedSerializeStream &out, const std::unique_ptr<WidgetBase> &widget) const;

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

        IntervalClock<> mFrameClock;

        struct WidgetManagerData;
        std::shared_ptr<WidgetManagerData> mData;

        // Dragging
        DragBeginEvent mDragStartEvent { { 0, 0 }, { 0, 0 }, Input::MouseButton::NO_BUTTON };
        bool mDragging = false;
        bool mDraggingAborted = false;
        std::chrono::steady_clock::time_point mDragStartTime;
    };

}
}