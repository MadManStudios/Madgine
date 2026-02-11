#include "../widgetslib.h"

#include "image.h"

#include "Madgine/imageloader/imageloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "widgetmanager.h"

NAMED_UNIQUECOMPONENT(Image, Engine::Widgets::Image);

METATABLE_BEGIN_BASE(Engine::Widgets::Image, Engine::Widgets::WidgetBase)
    NAMED_MEMBER(Image, mImageRenderData)
    MEMBER(mColor)
METATABLE_END(Engine::Widgets::Image)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::Image, Engine::Widgets::WidgetBase)
    FIELD(mImageRenderData)
    FIELD(mColor)
SERIALIZETABLE_END(Engine::Widgets::Image)

namespace Engine {
namespace Widgets {

    void Image::render(WidgetsRenderData &renderData)
    {
        const Atlas2::Entry *entry = manager().lookUpImage(mImageRenderData.image());

        if (entry) {
            Vector2 pos = getAbsolutePosition();
            Vector3 size = getAbsoluteSize();

            mImageRenderData.renderImage(renderData, pos, size, *entry, mColor.frame(pos, size.xy()));
        }

        WidgetBase::render(renderData);
    }

}
}
