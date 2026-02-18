#include "../widgetslib.h"

#include "widgetmanager.h"

#include "Interfaces/input/inputevents.h"
#include "Interfaces/window/windowapi.h"

#include "Meta/serialize/helper/typedobjectserialize.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Madgine/imageloader/imagedata.h"
#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/meshloader/meshloader.h"
#include "Madgine/render/fonts/fontloader.h"
#include "Madgine/render/pipelineloader.h"
#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"
#include "Madgine/window/mainwindow.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "atlasloader.h"
#include "widget.h"
#include "widgetloader.h"
#include "widgets_hlsl.h"

NAMED_UNIQUECOMPONENT(WidgetManager, Engine::Widgets::WidgetManager)

METATABLE_BEGIN(Engine::Widgets::WidgetManager)
    READONLY_PROPERTY(Widgets, widgets)
    MEMBER(mWidgetsLayout)
    MEMBER(mShadowOffset)
METATABLE_END(Engine::Widgets::WidgetManager)

SERIALIZETABLE_BEGIN(Engine::Widgets::WidgetManager)
    FIELD(mWidgetsLayout)
SERIALIZETABLE_END(Engine::Widgets::WidgetManager)

namespace Engine {
namespace Widgets {

    static float sDragDistanceThreshold = 5.0f;
    static std::chrono::steady_clock::duration sDragTimeThreshold = 20ms;

    struct WidgetManager::WidgetManagerData {
        UIAtlas mAtlas;
    };

    WidgetManager::WidgetManager(Window::MainWindow &window)
        : MainWindowComponent(window, 20)
        , mLifetime(&window.lifetime())        
        , mData(std::make_shared<WidgetManagerData>())
    {
    }

    WidgetManager::WidgetManager(const WidgetManager &sharedInstance)
        : MainWindowComponent(sharedInstance.mWindow, 20)
        , mLifetime(&mWindow.lifetime())
        , mData(sharedInstance.mData)
    {
    }

    WidgetManager::~WidgetManager()
    {
        assert(mWidgets.empty());
    }

    Threading::Task<bool> WidgetManager::init()
    {
        if (!co_await MainWindowComponentBase::init())
            co_return false;

#ifdef MADGINE_MAINWINDOW_LAYOUT
        AtlasLoader::Handle atlas;
        bool hasAtlas = co_await atlas.load(STRINGIFY2(MADGINE_MAINWINDOW_LAYOUT));
        if (hasAtlas) {
            mData->mAtlas.preload(*atlas);
        }
#endif

        for (auto &[name, res] : WidgetLoader::getSingleton()) {
            co_await res.loadData().info()->loadingTask();
        }

        startLifetime();

        co_return true;
    }

    Threading::Task<void> WidgetManager::finalize()
    {
        mTopLevelWidgets.clear();

        mData->mAtlas.reset();

        co_await MainWindowComponentBase::finalize();

        co_return;
    }

    void WidgetManager::startLifetime()
    {
        mWindow.lifetime().attach(mLifetime | Behavior::with_named<"WidgetManager">(this));
    }

    void WidgetManager::endLifetime()
    {
        mLifetime.end();
    }

    WidgetBase *WidgetManager::createTopLevel()
    {
        std::unique_ptr<WidgetBase> p = std::make_unique<WidgetBase>(*this);
        WidgetBase *w = p.get();

        mTopLevelWidgets.emplace_back(std::move(p));
        return w;
    }

    Serialize::StreamResult WidgetManager::readWidget(Serialize::CallerHierarchyFormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget, WidgetBase *parent)
    {
        std::string _class;
        STREAM_PROPAGATE_ERROR(Serialize::beginExtendedTypedRead(in, _class));

        if (_class == "Widget") {
            widget = std::make_unique<WidgetBase>(*this, parent);
        } else {
            auto it = WidgetRegistry::sComponentsByName().find(_class);
            if (it != WidgetRegistry::sComponentsByName().end()) {
                widget = construct(WidgetRegistry::get(it->second), *this, std::move(parent));
            } else {

                WidgetLoader::Handle desc;
                Threading::TaskFuture<bool> result = desc.load(_class);

                if (result) {
                    widget = desc.create(*this, parent);
                } else {
                    return STREAM_UNKNOWN_ERROR(in) << "Widget class '" << _class << "' not found!";
                }
            }
        }

        return {};
    }

    Serialize::StreamResult WidgetManager::readWidgetStub(Serialize::CallerHierarchyFormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget)
    {
        return readWidget(in, widget, nullptr);
    }

    const char *WidgetManager::writeWidget(Serialize::CallerHierarchyFormattedSerializeStream &out, const std::unique_ptr<WidgetBase> &widget) const
    {
        return Serialize::beginExtendedTypedWrite(out, widget->getClass());
    }

    Serialize::StreamResult WidgetManager::scanWidget(const Serialize::SerializeTable *&out, Serialize::CallerHierarchyFormattedSerializeStream &in)
    {
        std::string _class;
        STREAM_PROPAGATE_ERROR(Serialize::beginExtendedTypedRead(in, _class));

        if (_class == "Widget") {
            out = &serializeTable<WidgetBase>();
        } else {
            auto it = WidgetRegistry::sComponentsByName().find(_class);
            if (it != WidgetRegistry::sComponentsByName().end()) {
                out = WidgetRegistry::get(it->second).mType;
            } else {
                WidgetLoader::Resource *res = WidgetLoader::get(_class);
                if (res) {
                    out = &serializeTable<CompoundWidget>();
                } else {
                    throw 0;
                }
            }
        }

        return {};
    }

    bool WidgetManager::onWindowEvent(const Window::WindowEvent &arg)
    {
        return std::visit(overloaded {
                              [&](const Window::ResizeEvent &e) { return false; },
                              [&](const Window::CloseEvent &e) { return false; },
                              [&](const Window::RepaintEvent &e) { return false; },
                              [&](const Input::KeyPressEvent &e) { return injectKeyPress(e); },
                              [&](const Input::KeyReleaseEvent &e) { return injectKeyRelease(e); },
                              [&](const Input::PointerPressEvent &e) { return injectPointerPress(e); },
                              [&](const Input::PointerReleaseEvent &e) { return injectPointerRelease(e); },
                              [&](const Input::PointerMoveEvent &e) { return injectPointerMove(e); },
                              [&](const Input::AxisEvent &e) { return injectAxisEvent(e); } },
            arg);
    }

    bool WidgetManager::injectPointerPress(const Input::PointerPressEvent &arg)
    {
        assert(mDragStartEvent.mButton != arg.mButton);
        if (mDragStartEvent.mButton != Input::MouseButton::NO_BUTTON)
            return true;

        if (mPointerEventTargetWidget) {
            if (mFocusedWidget != mPointerEventTargetWidget) {
                if (mFocusedWidget) {
                    mFocusedWidget->onFocusLost();
                }
                mFocusedWidget = mPointerEventTargetWidget;
            }

            mDragStartEvent = DragBeginEvent { arg.mWindowPosition, arg.mScreenPosition, arg.mButton };

            Vector2i pos = mFocusedWidget->getAbsolutePosition().floor();
            mDragStartEvent.mWindowPosition = mDragStartEvent.mWindowPosition - InterfacesVector { pos.x, pos.y };

            mDragStartTime = std::chrono::steady_clock::now();

            return true;
        } else {
            if (mFocusedWidget) {
                mFocusedWidget->onFocusLost();
            }
            mFocusedWidget = nullptr;
        }

        return false;
    }

    bool WidgetManager::injectKeyPress(const Input::KeyPressEvent &arg)
    {
        for (WidgetBase *modalWidget : mModalWidgetList) {
            while (modalWidget) {
                if (modalWidget->injectKeyPress(arg))
                    return true;
                modalWidget = modalWidget->getParent();
            }
        }

        WidgetBase *w = mFocusedWidget;
        while (w) {
            if (w->injectKeyPress(arg))
                return true;
            w = w->getParent();
        }

        return false;
    }

    bool WidgetManager::injectKeyRelease(const Input::KeyReleaseEvent &arg)
    {
        for (WidgetBase *modalWidget : mModalWidgetList) {
            while (modalWidget) {
                if (modalWidget->injectKeyRelease(arg))
                    return true;
                modalWidget = modalWidget->getParent();
            }
        }

        WidgetBase *w = mFocusedWidget;
        while (w) {
            if (w->injectKeyRelease(arg))
                return true;
            w = w->getParent();
        }

        return false;
    }

    bool WidgetManager::injectPointerRelease(const Input::PointerReleaseEvent &arg)
    {
        if (mDragStartEvent.mButton != arg.mButton)
            return false;

        if (mFocusedWidget) {

            Vector2i pos = mFocusedWidget->getAbsolutePosition().floor();
            arg.mWindowPosition = arg.mWindowPosition - InterfacesVector { pos.x, pos.y };
            if (mDragging) {
                if (!mDraggingAborted)
                    mFocusedWidget->injectDragEnd(DragEndEvent { arg.mWindowPosition, arg.mScreenPosition, arg.mButton });
                mDragging = false;
            } else {
                mFocusedWidget->injectPointerClick(PointerClickEvent { arg.mWindowPosition, arg.mScreenPosition, arg.mButton });
            }

            mDragStartEvent.mButton = Input::MouseButton::NO_BUTTON;

            return true;
        }

        return false;
    }

    WidgetBase *WidgetManager::getHoveredWidgetUp(const Vector2 &pos, WidgetBase *current)
    {
        return current ? current->getHoveredUp(pos, { { 0, 0 }, mClientSpace.mSize }) : nullptr;
    }

    WidgetBase *WidgetManager::getHoveredWidgetDown(const Vector2 &pos, WidgetBase *current)
    {
        if (current) {
            return current->getHoveredDown(pos, { { 0, 0 }, mClientSpace.mSize });
        } else {
            if (!mModalWidgetList.empty()) {
                assert(mModalWidgetList.front()->mVisible);
                return mModalWidgetList.front()->getHoveredDown(pos, { { 0, 0 }, mClientSpace.mSize });
            }
            for (WidgetBase *w : widgets()) {
                if (WidgetBase *hovered = w->getHoveredDown(pos, { { 0, 0 }, mClientSpace.mSize })) {
                    return hovered;
                }
            }
            return nullptr;
        }
    }

    WidgetBase *WidgetManager::getHoveredWidget(const Vector2 &pos, WidgetBase *current)
    {
        return getHoveredWidgetDown(pos, getHoveredWidgetUp(pos, current));
    }

    bool WidgetManager::injectPointerMove(const Input::PointerMoveEvent &arg)
    {
        if (std::ranges::find(mWidgets, mHoveredWidget) == mWidgets.end())
            mHoveredWidget = nullptr;

        if (mDragStartEvent.mButton != Input::MouseButton::NO_BUTTON) {

            if (!mDragging && mFocusedWidget->allowsDragging()) {
                InterfacesVector dist = arg.mScreenPosition - mDragStartEvent.mScreenPosition;
                if (std::abs(dist.x) + std::abs(dist.y) > sDragDistanceThreshold && std::chrono::steady_clock::now() - mDragStartTime > sDragTimeThreshold) {
                    mDragging = true;
                    mDraggingAborted = false;
                    mFocusedWidget->injectDragBegin(mDragStartEvent);
                }
            }

            if (mDragging && !mDraggingAborted) {
                Vector2i pos = mFocusedWidget->getAbsolutePosition().floor();
                arg.mWindowPosition = arg.mWindowPosition - InterfacesVector { pos.x, pos.y };
                mFocusedWidget->injectDragMove(DragMoveEvent { arg.mWindowPosition, arg.mScreenPosition, arg.mMoveDelta });
            }

            return false;
        }

        WidgetBase *hoveredWidget = getHoveredWidget(Vector2 { Vector2i { &arg.mWindowPosition.x } }, mHoveredWidget);

        bool enter = false;
        if (mHoveredWidget != hoveredWidget) {

            if (mHoveredWidget) {
                InterfacesVector storedWindowPosition = arg.mWindowPosition;
                Vector2i pos = mHoveredWidget->getAbsolutePosition().floor();
                arg.mWindowPosition = arg.mWindowPosition - InterfacesVector { pos.x, pos.y };
                mHoveredWidget->injectPointerLeave(arg);
                arg.mWindowPosition = storedWindowPosition;
            }

            mHoveredWidget = hoveredWidget;
            enter = true;

            mPointerEventTargetWidget = hoveredWidget;
            while (mPointerEventTargetWidget && !mPointerEventTargetWidget->acceptsPointerEvents()) {
                mPointerEventTargetWidget = mPointerEventTargetWidget->getParent();
            }
        }

        if (mPointerEventTargetWidget) {
            Vector2i pos = mPointerEventTargetWidget->getAbsolutePosition().floor();
            arg.mWindowPosition = arg.mWindowPosition - InterfacesVector { pos.x, pos.y };

            if (enter)
                mPointerEventTargetWidget->injectPointerEnter(arg);

            mPointerEventTargetWidget->injectPointerMove(arg);
            return true;
        }

        return false;
    }

    bool WidgetManager::injectAxisEvent(const Input::AxisEvent &arg)
    {
        if (std::ranges::find(mWidgets, mHoveredWidget) == mWidgets.end())
            mHoveredWidget = nullptr;

        if (mHoveredWidget)
            return mHoveredWidget->injectAxisEvent(arg);

        return false;
    }

    WidgetBase *WidgetManager::currentRoot()
    {
        return mCurrentRoot;
    }

    void WidgetManager::destroyTopLevel(WidgetBase *w)
    {
        if (mCurrentRoot == w)
            mCurrentRoot = nullptr;
        auto it = std::ranges::find(mTopLevelWidgets, w, projectionToRawPtr);
        assert(it != mTopLevelWidgets.end());
        mTopLevelWidgets.erase(it);
    }

    void WidgetManager::clear()
    {
        mTopLevelWidgets.clear();
    }

    bool WidgetManager::isHovered(WidgetBase *w)
    {
        WidgetBase *hovered = mHoveredWidget;
        while (hovered) {
            if (hovered == w)
                return true;
            hovered = hovered->getParent();
        }
        return false;
    }

    WidgetBase *WidgetManager::hoveredWidget()
    {
        return mHoveredWidget;
    }

    WidgetBase *WidgetManager::focusedWidget()
    {
        return mFocusedWidget;
    }

    WidgetBase *WidgetManager::pointerEventTargetWidget()
    {
        return mPointerEventTargetWidget;
    }

    WidgetBase *WidgetManager::getWidget(std::string_view name)
    {
        auto it = std::ranges::find(mWidgets, name, &WidgetBase::mName);
        if (it == mWidgets.end())
            return nullptr;
        return *it;
    }

    void WidgetManager::registerWidget(WidgetBase *w)
    {
        mWidgets.push_back(w);
    }

    void WidgetManager::unregisterWidget(WidgetBase *w)
    {
        /* auto count = */ std::erase(mWidgets, w);
        // assert(count == 1);
    }

    void WidgetManager::resetPointerState()
    {
        if (mFocusedWidget) {
            mFocusedWidget->onFocusLost();
        }
        mFocusedWidget = nullptr;
        mHoveredWidget = nullptr;
        if (mPointerEventTargetWidget) {
            Input::PointerMoveEvent arg {
                { 0, 0 }, { 0, 0 }, { 0, 0 }
            };
            mPointerEventTargetWidget->injectPointerLeave(arg);
            mPointerEventTargetWidget = nullptr;
        }
    }

    void WidgetManager::swapCurrentRoot(std::string_view name)
    {
        auto it = std::ranges::find(mTopLevelWidgets, name, &WidgetBase::mName);
        if (it != mTopLevelWidgets.end())
            swapCurrentRoot(it->get());
    }

    void WidgetManager::swapCurrentRoot(WidgetBase *newRoot)
    {
        if (mCurrentRoot)
            mCurrentRoot->hide();

        resetPointerState();

        mCurrentRoot = newRoot;
        newRoot->show();
    }

    void WidgetManager::openModalWidget(WidgetBase *widget)
    {
        resetPointerState();

        mModalWidgetList.emplace(mModalWidgetList.begin(), widget);
        widget->show();
    }

    void WidgetManager::openWidget(WidgetBase *widget)
    {
        widget->show();
    }

    void WidgetManager::closeModalWidget(WidgetBase *widget)
    {
        resetPointerState();

        assert(mModalWidgetList.size() > 0 && mModalWidgetList.front() == widget);
        widget->hide();
        mModalWidgetList.erase(mModalWidgetList.begin());
    }

    void WidgetManager::closeWidget(WidgetBase *widget)
    {
        widget->hide();
    }

    void WidgetManager::openOverlay(WidgetBase *widget)
    {
        mOverlays.push_back(widget);
        widget->show();
        widget->applyGeometry(Vector3 { Vector2 { mClientSpace.mSize }, Window::platformCapabilities.mScalingFactor });
    }

    void WidgetManager::closeOverlay(WidgetBase *widget)
    {
        widget->hide();
        std::erase(mOverlays, widget);
    }

    void WidgetManager::openLayout(std::string_view name)
    {
        auto it = std::ranges::find(mWidgetsLayout, name, &LayoutWidget::mName);
        if (it != mWidgetsLayout.end() && it->mWidget.isSet()) {
            switch (it->mType) {
            case WidgetType::MODAL_OVERLAY:
                openModalWidget(std::get<0>(*it->mWidget));
                break;
            case WidgetType::DEFAULT_WIDGET:
                openWidget(std::get<0>(*it->mWidget));
                break;
            case WidgetType::NONMODAL_OVERLAY:
                openWidget(std::get<0>(*it->mWidget));
                break;
            case WidgetType::ROOT_WIDGET:
                swapCurrentRoot(std::get<0>(*it->mWidget));
                break;
            }
        }
    }

    void WidgetManager::closeLayout(std::string_view name)
    {
        auto it = std::ranges::find(mWidgetsLayout, name, &LayoutWidget::mName);
        if (it != mWidgetsLayout.end()) {
            switch (it->mType) {
            case WidgetType::MODAL_OVERLAY:
                closeModalWidget(std::get<0>(*it->mWidget));
                break;
            case WidgetType::DEFAULT_WIDGET:
                closeWidget(std::get<0>(*it->mWidget));
                break;
            case WidgetType::NONMODAL_OVERLAY:
                closeWidget(std::get<0>(*it->mWidget));
                break;
            case WidgetType::ROOT_WIDGET:
                // swapCurrentRoot(it->mWidget);
                break;
            }
        }
    }

    void WidgetManager::createLayout(std::string_view name)
    {
        mWidgetsLayout.emplace_back().mName = name;
    }

    LayoutWidget *WidgetManager::getLayoutWidget(std::string_view name)
    {
        auto it = std::ranges::find(mWidgetsLayout, name, &LayoutWidget::mName);
        return it == mWidgetsLayout.end() ? nullptr : &*it;
    }

    std::list<LayoutWidget> &WidgetManager::layoutWidgets()
    {
        return mWidgetsLayout;
    }

    void WidgetManager::onResize(const Rect2i &space)
    {
        MainWindowComponentBase::onResize(space);
        for (WidgetBase *topLevel : widgets()) {
            topLevel->applyGeometry(Vector3 { Vector2 { space.mSize }, Window::platformCapabilities.mScalingFactor });
        }
    }

    void WidgetManager::render(Render::RenderTarget *target, size_t iteration)
    {
        mFrameClock.tick(std::chrono::steady_clock::now());

        MainWindowComponentBase::render(target, iteration);

        WidgetsRenderData renderData;
        auto keep = renderData.pushClipRect(Vector2::ZERO, Vector2 { mClientSpace.mSize });

        for (LayoutWidget &layoutWidget : mWidgetsLayout) {
            if (!layoutWidget.mWidget.isSet() && layoutWidget.mWidgetTemplate && layoutWidget.mWidgetTemplate.info()->loadingTask().is_ready() && layoutWidget.mWidgetTemplate.info()->loadingTask()) {
                std::unique_ptr<WidgetBase> p = layoutWidget.mWidgetTemplate.create(*this);

                layoutWidget.mWidget.emplace(p.get());
                p->applyGeometry(Vector3 { Vector2 { mClientSpace.mSize }, Window::platformCapabilities.mScalingFactor });
                mTopLevelWidgets.emplace_back(std::move(p));

                if (layoutWidget.mDefaultVisibility) {
                    openLayout(layoutWidget.mName);
                }
            }
        }

        if (mCurrentRoot) {
            renderData.setAlpha(mCurrentRoot->opacity());
            renderData.setLayer(0);
            mCurrentRoot->render(renderData);
        }

        int layer = 0;
        for (Widgets::WidgetBase *w : mModalWidgetList) {
            if (w->mVisible) {
                renderData.setAlpha(w->opacity());
                renderData.setLayer(20 * ++layer);
                w->render(renderData);
            }
        }
        ++layer;
        for (Widgets::WidgetBase *w : mOverlays) {
            if (w->mVisible) {
                renderData.setAlpha(w->opacity());
                renderData.setLayer(20 * layer);
                w->render(renderData);
            }
        }

        render(target, renderData, mClientSpace.mSize);
    }

    void WidgetManager::render(Render::RenderTarget *target, const WidgetsRenderData &renderData, const Vector2i &size)
    {
        if (!mPipeline.available())
            return;

        {
            auto perApp = mPipeline->mapParameters<HLSL::WidgetsPerApplication>(0);
            perApp->c = target->getClipSpaceMatrix();
            perApp->screenSize = Vector2 { size };
            perApp->distanceFieldScaling = 2.0f / Render::FontLoader::sFontSize * 64.0f;
        }

        for (auto &[layer, layerData] : renderData.vertexData()) {
            for (auto &[tex, vertexData] : layerData) {
                if (vertexData.mTriangleVertices.empty())
                    continue;

                {
                    auto parameters = mPipeline->mapParameters<HLSL::WidgetsPerObject>(2);
                    parameters->hasDistanceField = bool(tex.mFlags & TextureFlag_IsDistanceField);
                    parameters->hasTexture = true;
                    parameters->shadowOffset = mShadowOffset / Vector2 { target->size() };
                }

                {
                    auto vertices = mPipeline->mapVertices<Vertex[]>(target, vertexData.mTriangleVertices.size());
                    std::ranges::copy(vertexData.mTriangleVertices, vertices.mData);
                }

                if (tex.mResource)
                    mPipeline->bindResources(target, 2, tex.mResource);
                else
                    mPipeline->bindResources(target, 2, mData->mAtlas.resource());

                mPipeline->setGroupSize(3);
                mPipeline->render(target);
            }
        }
        if (!renderData.lineVertices().empty()) {
            {
                auto parameters = mPipeline->mapParameters<HLSL::WidgetsPerObject>(2);
                parameters->hasDistanceField = false;
                parameters->hasTexture = false;
                parameters->shadowOffset = { 0, 0 };
            }

            if (mData->mAtlas.resource())
                mPipeline->bindResources(target, 2, mData->mAtlas.resource());

            {
                auto vertices = mPipeline->mapVertices<Vertex[]>(target, renderData.lineVertices().size());
                std::ranges::copy(renderData.lineVertices(), vertices.mData);
            }

            mPipeline->setGroupSize(2);
            mPipeline->render(target);
        }
    }

    Resources::ImageLoader::Resource *WidgetManager::getImage(std::string_view name)
    {
        return mData->mAtlas.getImage(name);
    }

    const Atlas2::Entry *WidgetManager::lookUpImage(Resources::ImageLoader::Resource *image)
    {
        return mData->mAtlas.lookUpImage(image);
    }

    const Atlas2::Entry *WidgetManager::lookUpImage(std::string_view name)
    {
        return mData->mAtlas.lookUpImage(name);
    }

    void WidgetManager::onActivate(Serialize::CallbackTiming timing, bool active)
    {
        if ((!active && timing == Serialize::CallbackTiming::AFTER) || (active && timing == Serialize::CallbackTiming::BEFORE))
            return;

        if (active) {
            for (LayoutWidget &layoutWidget : mWidgetsLayout) {
                if (!layoutWidget.mWidget.isSet() && layoutWidget.mWidgetTemplate.available()) {
                    std::unique_ptr<WidgetBase> p = layoutWidget.mWidgetTemplate.create(*this);
                    layoutWidget.mWidget.emplace(p.get());
                    p->applyGeometry(Vector3 { Vector2 { mClientSpace.mSize }, Window::platformCapabilities.mScalingFactor });
                    mTopLevelWidgets.emplace_back(std::move(p));
                    if (layoutWidget.mDefaultVisibility) {
                        openLayout(layoutWidget.mName);
                    }
                }
            }
        } else {
            for (LayoutWidget &layoutWidget : mWidgetsLayout) {
                if (layoutWidget.mWidget.isSet()) {
                    destroyTopLevel(std::get<0>(*layoutWidget.mWidget));
                    layoutWidget.mWidget.reset();
                }
            }
        }
    }

    bool WidgetManager::dragging(const WidgetBase *widget)
    {
        return mFocusedWidget == widget && mDragging && !mDraggingAborted;
    }

    void WidgetManager::abortDrag(WidgetBase *widget)
    {
        if (dragging(widget)) {
            mFocusedWidget->injectDragAbort();
            mDraggingAborted = true;
        }
    }

    Debug::DebuggableLifetime<Behavior::get_named_d> &WidgetManager::lifetime()
    {
        return mLifetime;
    }

    IntervalClock<> &WidgetManager::clock()
    {
        return mFrameClock;
    }

    void WidgetManager::setup(Render::RenderTarget *target)
    {
        setupImpl(target, HLSL::widgets_VS, HLSL::widgets_PS, { sizeof(HLSL::WidgetsPerApplication), 0, sizeof(HLSL::WidgetsPerObject) });
    }
}
}