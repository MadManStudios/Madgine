#include "../widgetslib.h"

#include "scenewindow.h"

#include "widgetmanager.h"

#include "Meta/math/vector4.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/render/rendertarget.h"

#include "Madgine/render/texture.h"

#include "util/renderdata.h"

METATABLE_BEGIN_BASE(Engine::Widgets::SceneWindow, Engine::Widgets::WidgetBase)
METATABLE_END(Engine::Widgets::SceneWindow)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::SceneWindow, Engine::Widgets::WidgetBase)
SERIALIZETABLE_END(Engine::Widgets::SceneWindow)

namespace Engine {
namespace Widgets {

    SceneWindow::SceneWindow(WidgetManager &manager, WidgetBase *parent)
        : Widget(manager, parent)
    {
    }

    SceneWindow::~SceneWindow()
    {
        if (mSource)
            manager().removeDependency(mSource);
    }

    void SceneWindow::render(WidgetsRenderData &renderData)
    {
        if (mSource) {

            Vector2 pos = getAbsolutePosition();
            Vector3 size = getAbsoluteSize();

            renderData.renderQuad(pos, size.xy(), { 1.0f, 1.0f, 1.0f, 1.0f }, { mSource->texture()->resourceBlock() });
        }

        WidgetBase::render(renderData);
    }

    void SceneWindow::setRenderSource(Render::RenderTarget *source)
    {
        if (mSource)
            manager().removeDependency(mSource);
        mSource = source;
        if (mSource)
            manager().addDependency(mSource);
        mSource->resize(getAbsoluteSize().xy().floor());
    }

    void SceneWindow::sizeChanged(const Vector3 &pixelSize)
    {
        if (mSource)
            mSource->resize(pixelSize.xy().floor());
    }

    std::string SceneWindow::getClass() const
    {
        return "SceneWindow";
    }

}
}
