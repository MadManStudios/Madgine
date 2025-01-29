#include "../widgetslib.h"

#include "image.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/imageloader/imageloader.h"

#include "widgetmanager.h"


METATABLE_BEGIN_BASE(Engine::Widgets::Image, Engine::Widgets::WidgetBase)
NAMED_MEMBER(Image, mImageRenderData)
METATABLE_END(Engine::Widgets::Image)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::Image, Engine::Widgets::WidgetBase)
FIELD(mImageRenderData)
SERIALIZETABLE_END(Engine::Widgets::Image)

namespace Engine {
namespace Widgets {

    void Image::vertices(WidgetsRenderData &renderData, size_t layer)
    {
        const Atlas2::Entry* entry = manager().lookUpImage(mImageRenderData.image());
        if (!entry)
            return;

        Vector3 pos { getAbsolutePosition(), static_cast<float>(depth(layer)) };
        Vector3 size = getAbsoluteSize();

        mImageRenderData.renderImage(renderData, pos, size.xy(), *entry);
    }

    WidgetClass Image::getClass() const
    {
        return WidgetClass::IMAGE;
    }

}
}
