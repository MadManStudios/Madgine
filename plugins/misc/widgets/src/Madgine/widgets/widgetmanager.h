#pragma once

#include "Madgine/window/mainwindowcomponent.h"
#include "Madgine/window/mainwindowcomponentcollector.h"

#include "Madgine/render/renderpass.h"

#include "Generic/execution/signal.h"

#include "Generic/coroutines/generator.h"

#include "Meta/math/atlas2.h"

#include "Madgine/imageloader/imageloader.h"

#include "Interfaces/input/inputevents.h"

#include "Generic/projections.h"

#include "Madgine/debug/debuggablelifetime.h"

#include "Madgine/bindings.h"

#include "Generic/intervalclock.h"

namespace Engine {
namespace Widgets {

    struct MADGINE_WIDGETS_EXPORT WidgetManager : Window::MainWindowComponent<WidgetManager> {

        SERIALIZABLEUNIT(WidgetManager)

        WidgetManager(Window::MainWindow &window);
        WidgetManager(const WidgetManager &) = delete;
        ~WidgetManager();

        void swapCurrentRoot(std::string_view name);
        void swapCurrentRoot(WidgetBase *newRoot);
        void openModalWidget(WidgetBase *widget);
        void closeModalWidget(WidgetBase *widget);
        void openWidget(WidgetBase *widget);
        void closeWidget(WidgetBase *widget);
        void openOverlay(WidgetBase *widget);
        void closeOverlay(WidgetBase *widget);

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

        Generator<std::pair<WidgetBase *, size_t>> visibleWidgets();

        WidgetBase *mStartupWidget = nullptr;
        void openStartupWidget();

        virtual bool injectPointerPress(const Input::PointerEventArgs &arg) override;
        virtual bool injectPointerRelease(const Input::PointerEventArgs &arg) override;
        virtual bool injectPointerMove(const Input::PointerEventArgs &arg) override;
        virtual bool injectAxisEvent(const Input::AxisEventArgs &arg) override;
        virtual bool injectKeyPress(const Input::KeyEventArgs &arg) override;

        virtual void onResize(const Rect2i &space) override;
        virtual void render(Render::RenderTarget *target, size_t iteration) override;

        Resources::ImageLoader::Resource *getImage(std::string_view name);

        const Atlas2::Entry *lookUpImage(Resources::ImageLoader::Resource *image);
        const Atlas2::Entry *lookUpImage(std::string_view name);

        bool dragging(const WidgetBase *widget);
        void abortDrag(WidgetBase *widget);

        Debug::DebuggableLifetime<get_binding_d> &lifetime();

        IntervalClock<> &clock();

        using RenderPass::addDependency;
        using RenderPass::removeDependency;

    protected:
        WidgetBase *getHoveredWidget(const Vector2 &pos, WidgetBase *current);
        WidgetBase *getHoveredWidgetUp(const Vector2 &pos, WidgetBase *current);
        WidgetBase *getHoveredWidgetDown(const Vector2 &pos, WidgetBase *current);

        void resetPointerState();

        std::unique_ptr<WidgetBase> createWidgetByDescriptor(const WidgetDescriptor &desc, WidgetBase *parent = nullptr);
        Serialize::StreamResult readWidget(Serialize::FormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget, WidgetBase *parent);
        Serialize::StreamResult readWidgetStub(Serialize::FormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget);
        const char *writeWidget(Serialize::FormattedSerializeStream &out, const std::unique_ptr<WidgetBase> &widget) const;
        static Serialize::StreamResult scanWidget(const Serialize::SerializeTable *&out, Serialize::FormattedSerializeStream &in);

        template <typename WidgetType = WidgetBase>
        std::unique_ptr<WidgetType> create(WidgetBase *parent = nullptr);

        friend struct WidgetBase;

        void onActivate(bool active);

    private:
        std::vector<WidgetBase *> mWidgets;

        std::vector<std::unique_ptr<WidgetBase>> mTopLevelWidgets;

        WidgetBase *mHoveredWidget = nullptr;
        WidgetBase *mFocusedWidget = nullptr;

        WidgetBase *mPointerEventTargetWidget = nullptr;

        WidgetBase *mCurrentRoot = nullptr;

        std::vector<WidgetBase *> mModalWidgetList;
        std::vector<WidgetBase *> mOverlays;

        DEBUGGABLE_LIFETIME(mLifetime, get_binding_d);

        IntervalClock<> mFrameClock;

        struct WidgetManagerData;
        std::unique_ptr<WidgetManagerData> mData;

        //Dragging
        Input::PointerEventArgs mDragStartEvent { { 0, 0 }, { 0, 0 }, Input::MouseButton::NO_BUTTON };
        bool mDragging = false;
        bool mDraggingAborted = false;
        std::chrono::steady_clock::time_point mDragStartTime;
    };

}
}

REGISTER_TYPE(Engine::Widgets::WidgetManager)