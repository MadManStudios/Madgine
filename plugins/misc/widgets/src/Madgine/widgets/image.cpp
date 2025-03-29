#include "../widgetslib.h"

#include "image.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/imageloader/imageloader.h"

#include "widgetmanager.h"


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
        const Atlas2::Entry* entry = manager().lookUpImage(mImageRenderData.image());
        if (!entry)
            return;

        Vector2 pos = getAbsolutePosition();
        Vector3 size = getAbsoluteSize();

        mImageRenderData.renderImage(renderData, pos, size.xy(), *entry, mColor);

        WidgetBase::render(renderData);
    }

    std::string Image::getClass() const
    {
        return "Image";
    }

}
}
