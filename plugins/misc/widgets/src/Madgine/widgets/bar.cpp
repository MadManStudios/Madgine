#include "../widgetslib.h"

#include "bar.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/math/atlas2.h"

#include "widgetmanager.h"

METATABLE_BEGIN_BASE(Engine::Widgets::Bar, Engine::Widgets::WidgetBase)
MEMBER(mRatio)
MEMBER(mColor)
METATABLE_END(Engine::Widgets::Bar)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::Bar, Engine::Widgets::WidgetBase)
SERIALIZETABLE_END(Engine::Widgets::Bar)

namespace Engine {
namespace Widgets {

    void Bar::vertices(WidgetsRenderData &renderData, size_t layer)
    {
        

        Vector3 pos { getAbsolutePosition(), static_cast<float>(depth(layer)) };
        Vector3 size = getAbsoluteSize();

        Color4 color = mColor;

        const Atlas2::Entry *blankEntry = manager().lookUpImage("blank_white");

        if (blankEntry) {
            renderData.renderQuadUV(pos, { mRatio * size.x, size.y }, color, {}, blankEntry->mArea, { 2048, 2048 }, blankEntry->mFlipped);
        }

        
    }

    WidgetClass Bar::getClass() const
    {
        return WidgetClass::BAR;
    }

}
}
