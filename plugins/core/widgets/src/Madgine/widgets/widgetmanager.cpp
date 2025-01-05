#include "../widgetslib.h"

#include "widgetmanager.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/meshloader/gpumeshloader.h"
#include "Madgine/pipelineloader/pipelineloader.h"

#include "widget.h"

#include "bar.h"
#include "button.h"
#include "checkbox.h"
#include "combobox.h"
#include "image.h"
#include "label.h"
#include "scenewindow.h"
#include "tabbar.h"
#include "tablewidget.h"
#include "textbox.h"
#include "textedit.h"
#include "layout.h"

#include "Madgine/imageloader/imagedata.h"

#include "Madgine/window/mainwindow.h"

#include "Interfaces/input/inputevents.h"

#include "Madgine/meshloader/meshloader.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendertarget.h"

#include "Meta/serialize/helper/typedobjectserialize.h"

#include "Madgine/render/shadinglanguage/sl_support_begin.h"
#include "shaders/widgets.sl"
#include "Madgine/render/shadinglanguage/sl_support_end.h"

#include "atlasloader.h"

#include "Interfaces/window/windowapi.h"

#include "Modules/threading/awaitables/awaitablesender.h"

NAMED_UNIQUECOMPONENT(WidgetManager, Engine::Widgets::WidgetManager)

METATABLE_BEGIN(Engine::Widgets::WidgetManager)
READONLY_PROPERTY(Widgets, widgets)
MEMBER(mStartupWidget)
METATABLE_END(Engine::Widgets::WidgetManager)

SERIALIZETABLE_BEGIN(Engine::Widgets::WidgetManager)
FIELD(mStartupWidget)
FIELD(mTopLevelWidgets, Serialize::ParentCreator<&Engine::Widgets::WidgetManager::readWidgetStub, &Engine::Widgets::WidgetManager::writeWidget, nullptr, &Engine::Widgets::WidgetManager::scanWidget>)
SERIALIZETABLE_END(Engine::Widgets::WidgetManager)

namespace Engine {
namespace Widgets {

    static constexpr const char *sTags[] = {
        "Widget",
        "SceneWindow",
        "Combobox",
        "Checkbox",
        "Textbox",
        "Button",
        "Label",
        "Bar",
        "Image",
        "TableWidget",
        "TabBar",
        "TextEdit",
        "Layout"
    };

    static float sDragDistanceThreshold = 5.0f;
    static std::chrono::steady_clock::duration sDragTimeThreshold = 20ms;

    struct WidgetManager::WidgetManagerData {

        Render::PipelineLoader::Instance mPipeline;

        UIAtlas mAtlas;
    };

    WidgetManager::WidgetManager(Window::MainWindow &window)
        : MainWindowComponent(window, 20)
        , mLifetime(&window.lifetime())
        , mData(std::make_unique<WidgetManagerData>())
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

        mData->mPipeline.create({ .vs = "widgets", .ps = "widgets", .bufferSizes = { sizeof(WidgetsPerApplication), 0, sizeof(WidgetsPerObject) } });

        if (!co_await mData->mAtlas.createTexture())
            co_return false;

#ifdef MADGINE_MAINWINDOW_LAYOUT
        AtlasLoader::Handle atlas;
        bool hasAtlas = co_await atlas.load(STRINGIFY2(MADGINE_MAINWINDOW_LAYOUT));
        if (hasAtlas) {
            mData->mAtlas.preload(*atlas);
        }
#endif

        startLifetime();

        co_return true;
    }

    Threading::Task<void> WidgetManager::finalize()
    {
        mTopLevelWidgets.clear();

        mData->mAtlas.reset();

        mData->mPipeline.reset();

        co_await MainWindowComponentBase::finalize();

        co_return;
    }

    void WidgetManager::startLifetime()
    {
        mWindow.lifetime().attach(mLifetime);
    }

    void WidgetManager::endLifetime()
    {
        mLifetime.end();
    }

    template <typename WidgetType>
    std::unique_ptr<WidgetType> WidgetManager::create(WidgetBase *parent)
    {
        std::unique_ptr<WidgetType> w = std::make_unique<WidgetType>(*this, parent);
        if (!parent) {
            w->hide();
        }
        w->applyGeometry(parent ? parent->getAbsoluteSize() : Vector3 { Vector2 { mClientSpace.mSize }, Window::platformCapabilities.mScalingFactor });
        return w;
    }

    template std::unique_ptr<WidgetBase> WidgetManager::create<WidgetBase>(WidgetBase *);
    template std::unique_ptr<Bar> WidgetManager::create<Bar>(WidgetBase *);
    template std::unique_ptr<Checkbox> WidgetManager::create<Checkbox>(WidgetBase *);
    template std::unique_ptr<Label> WidgetManager::create<Label>(WidgetBase *);
    template std::unique_ptr<Button> WidgetManager::create<Button>(WidgetBase *);
    template std::unique_ptr<Combobox> WidgetManager::create<Combobox>(WidgetBase *);
    template std::unique_ptr<Textbox> WidgetManager::create<Textbox>(WidgetBase *);
    template std::unique_ptr<SceneWindow> WidgetManager::create<SceneWindow>(WidgetBase *);
    template std::unique_ptr<Image> WidgetManager::create<Image>(WidgetBase *);
    template std::unique_ptr<TableWidget> WidgetManager::create<TableWidget>(WidgetBase *);
    template std::unique_ptr<TextEdit> WidgetManager::create<TextEdit>(WidgetBase *);
    template std::unique_ptr<Layout> WidgetManager::create<Layout>(WidgetBase *);

    template <typename WidgetType>
    WidgetType *WidgetManager::createTopLevel()
    {
        std::unique_ptr<WidgetType> p = create<WidgetType>();
        WidgetType *w = p.get();
        mTopLevelWidgets.emplace_back(std::move(p));
        return w;
    }

    template DLL_EXPORT WidgetBase *WidgetManager::createTopLevel<WidgetBase>();

    std::unique_ptr<WidgetBase> WidgetManager::createWidgetByClass(WidgetClass _class, WidgetBase *parent)
    {
        switch (_class) {
        case WidgetClass::WIDGET:
            return create<WidgetBase>(parent);
        case WidgetClass::BAR:
            return create<Bar>(parent);
        case WidgetClass::CHECKBOX:
            return create<Checkbox>(parent);
        case WidgetClass::LABEL:
            return create<Label>(parent);
        case WidgetClass::BUTTON:
            return create<Button>(parent);
        case WidgetClass::COMBOBOX:
            return create<Combobox>(parent);
        case WidgetClass::TEXTBOX:
            return create<Textbox>(parent);
        case WidgetClass::SCENEWINDOW:
            return create<SceneWindow>(parent);
        case WidgetClass::IMAGE:
            return create<Image>(parent);
        case WidgetClass::TABLEWIDGET:
            return create<TableWidget>(parent);
        case WidgetClass::TABBAR:
            return create<TabBar>(parent);
        case WidgetClass::TEXTEDIT:
            return create<TextEdit>(parent);
        case WidgetClass::LAYOUT:
            return create<Layout>(parent);
        default:
            std::terminate();
        }
    }

    Serialize::StreamResult WidgetManager::readWidget(Serialize::FormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget, WidgetBase *parent)
    {
        WidgetClass _class;
        STREAM_PROPAGATE_ERROR(Serialize::beginExtendedTypedRead(in, _class, sTags));

        widget = createWidgetByClass(_class, parent);
        return {};
    }

    Serialize::StreamResult WidgetManager::readWidgetStub(Serialize::FormattedSerializeStream &in, std::unique_ptr<WidgetBase> &widget)
    {
        return readWidget(in, widget, nullptr);
    }

    const char *WidgetManager::writeWidget(Serialize::FormattedSerializeStream &out, const std::unique_ptr<WidgetBase> &widget) const
    {
        return Serialize::beginExtendedTypedWrite(out, widget->getClass(), sTags);
    }

    Serialize::StreamResult WidgetManager::scanWidget(const Serialize::SerializeTable *&out, Serialize::FormattedSerializeStream &in)
    {
        WidgetClass _class;
        STREAM_PROPAGATE_ERROR(Serialize::beginExtendedTypedRead(in, _class, sTags));
        switch (_class) {
        case WidgetClass::WIDGET:
            out = &serializeTable<WidgetBase>();
            break;
        case WidgetClass::BAR:
            out = &serializeTable<Bar>();
            break;
        case WidgetClass::CHECKBOX:
            out = &serializeTable<Checkbox>();
            break;
        case WidgetClass::LABEL:
            out = &serializeTable<Label>();
            break;
        case WidgetClass::BUTTON:
            out = &serializeTable<Button>();
            break;
        case WidgetClass::COMBOBOX:
            out = &serializeTable<Combobox>();
            break;
        case WidgetClass::TEXTBOX:
            out = &serializeTable<Textbox>();
            break;
        case WidgetClass::SCENEWINDOW:
            out = &serializeTable<SceneWindow>();
            break;
        case WidgetClass::IMAGE:
            out = &serializeTable<Image>();
            break;
        case WidgetClass::TABLEWIDGET:
            out = &serializeTable<TableWidget>();
            break;
        case WidgetClass::TABBAR:
            out = &serializeTable<TabBar>();
            break;
        case WidgetClass::TEXTEDIT:
            out = &serializeTable<TextEdit>();
            break;
        case WidgetClass::LAYOUT:
            out = &serializeTable<Layout>();
            break;
        default:
            std::terminate();
        }
        return {};
    }

    bool WidgetManager::injectPointerPress(const Input::PointerEventArgs &arg)
    {
        assert(mDragStartEvent.button != arg.button);
        if (mDragStartEvent.button != Input::MouseButton::NO_BUTTON)
            return true;

        if (mPointerEventTargetWidget) {
            mFocusedWidget = mPointerEventTargetWidget;

            mDragStartEvent = arg;

            Vector2i pos = mFocusedWidget->getAbsolutePosition().floor();
            mDragStartEvent.windowPosition = mDragStartEvent.windowPosition - InterfacesVector { pos.x, pos.y };

            mDragStartTime = std::chrono::steady_clock::now();

            return true;
        } else {
            mFocusedWidget = nullptr;
        }

        return false;
    }

    bool WidgetManager::injectKeyPress(const Input::KeyEventArgs &arg)
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

    bool WidgetManager::injectPointerRelease(const Input::PointerEventArgs &arg)
    {
        if (mDragStartEvent.button != arg.button)
            return false;

        if (mFocusedWidget) {

            Vector2i pos = mFocusedWidget->getAbsolutePosition().floor();
            arg.windowPosition = arg.windowPosition - InterfacesVector { pos.x, pos.y };
            if (mDragging) {
                if (!mDraggingAborted)
                    mFocusedWidget->injectDragEnd(arg);
                mDragging = false;
            } else {
                mFocusedWidget->injectPointerClick(arg);
            }

            mDragStartEvent.button = Input::MouseButton::NO_BUTTON;

            return true;
        }

        return false;
    }

    WidgetBase *WidgetManager::getHoveredWidgetUp(const Vector2 &pos, WidgetBase *current)
    {
        if (!current) {
            return nullptr;
        } else if (!current->mVisible || !current->containsPoint(pos, { { 0, 0 }, mClientSpace.mSize })) {
            return getHoveredWidgetUp(pos, current->getParent());
        } else {
            return current;
        }
    }

    WidgetBase *WidgetManager::getHoveredWidgetDown(const Vector2 &pos, WidgetBase *current)
    {
        if (current) {
            for (WidgetBase *w : current->children()) {
                if (w->mVisible && w->containsPoint(pos, { { 0, 0 }, mClientSpace.mSize })) {
                    return getHoveredWidgetDown(pos, w);
                }
            }
        } else {
            if (!mModalWidgetList.empty()) {
                assert(mModalWidgetList.front()->mVisible);
                return getHoveredWidgetDown(pos, mModalWidgetList.front());
            }
            for (WidgetBase *w : widgets()) {
                if (w->mVisible && w->containsPoint(pos, { { 0, 0 }, mClientSpace.mSize })) {
                    return getHoveredWidgetDown(pos, w);
                }
            }
        }

        return current;
    }

    WidgetBase *WidgetManager::getHoveredWidget(const Vector2 &pos, WidgetBase *current)
    {
        return getHoveredWidgetDown(pos, getHoveredWidgetUp(pos, current));
    }

    bool WidgetManager::injectPointerMove(const Input::PointerEventArgs &arg)
    {
        if (std::ranges::find(mWidgets, mHoveredWidget) == mWidgets.end())
            mHoveredWidget = nullptr;

        if (mDragStartEvent.button != Input::MouseButton::NO_BUTTON) {

            if (!mDragging && mFocusedWidget->allowsDragging()) {
                InterfacesVector dist = arg.screenPosition - mDragStartEvent.screenPosition;
                if (std::abs(dist.x) + std::abs(dist.y) > sDragDistanceThreshold && std::chrono::steady_clock::now() - mDragStartTime > sDragTimeThreshold) {
                    mDragging = true;
                    mDraggingAborted = false;
                    mFocusedWidget->injectDragBegin(mDragStartEvent);
                }
            }

            if (mDragging && !mDraggingAborted) {
                Vector2i pos = mFocusedWidget->getAbsolutePosition().floor();
                arg.windowPosition = arg.windowPosition - InterfacesVector { pos.x, pos.y };
                mFocusedWidget->injectDragMove(arg);
            }

            return false;
        }

        WidgetBase *hoveredWidget = getHoveredWidget(Vector2 { Vector2i { &arg.windowPosition.x } }, mHoveredWidget);

        bool enter = false;
        if (mHoveredWidget != hoveredWidget) {

            if (mHoveredWidget) {
                InterfacesVector storedWindowPosition = arg.windowPosition;
                Vector2i pos = mHoveredWidget->getAbsolutePosition().floor();
                arg.windowPosition = arg.windowPosition - InterfacesVector { pos.x, pos.y };
                mHoveredWidget->injectPointerLeave(arg);
                arg.windowPosition = storedWindowPosition;
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
            arg.windowPosition = arg.windowPosition - InterfacesVector { pos.x, pos.y };

            if (enter)
                mPointerEventTargetWidget->injectPointerEnter(arg);

            mPointerEventTargetWidget->injectPointerMove(arg);
            return true;
        }

        return false;
    }

    bool WidgetManager::injectAxisEvent(const Input::AxisEventArgs &arg)
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
        //assert(count == 1);
    }

    void WidgetManager::resetPointerState()
    {
        mFocusedWidget = nullptr;
        mHoveredWidget = nullptr;
        if (mPointerEventTargetWidget) {
            Input::PointerEventArgs arg {
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

    void WidgetManager::openStartupWidget()
    {
        if (mStartupWidget)
            swapCurrentRoot(mStartupWidget);
        else if (mTopLevelWidgets.size() > 0)
            swapCurrentRoot(mTopLevelWidgets.front().get());
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
        if (!mData->mPipeline.available())
            return;

        MainWindowComponentBase::render(target, iteration);

        WidgetsRenderData renderData;
        auto keep = renderData.pushClipRect(Vector2::ZERO, Vector2 { mClientSpace.mSize });

        for (auto [w, layer] : visibleWidgets()) {
            w->vertices(renderData, layer);
        }

        {
            auto perApp = mData->mPipeline->mapParameters<WidgetsPerApplication>(0);
            perApp->c = target->getClipSpaceMatrix();
            perApp->screenSize = Vector2 { mClientSpace.mSize };
        }

        for (auto &[tex, vertexData] : renderData.vertexData()) {
            if (vertexData.mTriangleVertices.empty())
                continue;

            {
                auto parameters = mData->mPipeline->mapParameters<WidgetsPerObject>(2);
                parameters->hasDistanceField = bool(tex.mFlags & TextureFlag_IsDistanceField);
                parameters->hasTexture = true;
            }

            {
                auto vertices = mData->mPipeline->mapVertices<Vertex[]>(target, vertexData.mTriangleVertices.size());
                std::ranges::copy(vertexData.mTriangleVertices, vertices.mData);
            }

            if (tex.mResource)
                mData->mPipeline->bindResources(target, 2, tex.mResource);
            else
                mData->mPipeline->bindResources(target, 2, mData->mAtlas.resource());

            mData->mPipeline->setGroupSize(3);
            mData->mPipeline->render(target);
        }
        if (!renderData.lineVertices().empty()) {
            {
                auto parameters = mData->mPipeline->mapParameters<WidgetsPerObject>(2);
                parameters->hasDistanceField = false;
                parameters->hasTexture = false;
            }
             
            if (mData->mAtlas.resource())
                mData->mPipeline->bindResources(target, 2, mData->mAtlas.resource());

            {
                auto vertices = mData->mPipeline->mapVertices<Vertex[]>(target, renderData.lineVertices().size());
                std::ranges::copy(renderData.lineVertices(), vertices.mData);
            }

            mData->mPipeline->setGroupSize(2);
            mData->mPipeline->render(target);
        }
    }

    Generator<std::pair<WidgetBase *, size_t>> WidgetManager::visibleWidgets()
    {
        std::queue<std::pair<Widgets::WidgetBase *, size_t>> q;
        if (mCurrentRoot)
            q.emplace(mCurrentRoot, 0);
        int layer = 0;
        for (Widgets::WidgetBase *w : mModalWidgetList) {
            if (w->mVisible) {
                q.emplace(w, ++layer);
            }
        }
        while (!q.empty()) {
            auto [w, l] = q.front();
            q.pop();

            for (Widgets::WidgetBase *c : w->children()) {
                if (c->mVisible)
                    q.emplace(c, l);
            }

            co_yield { w, l };
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

    void WidgetManager::onActivate(bool active)
    {
        if (active) {
            for (WidgetBase *topLevel : widgets()) {
                topLevel->applyGeometry(Vector3 { Vector2 { getScreenSpace().mSize }, Window::platformCapabilities.mScalingFactor });
            }
            openStartupWidget();
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

    Debug::DebuggableLifetime<> &WidgetManager::lifetime()
    {
        return mLifetime;
    }

}
}