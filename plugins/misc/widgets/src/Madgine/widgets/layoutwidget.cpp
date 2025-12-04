#include "../widgetslib.h"

#include "layoutwidget.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN(Engine::Widgets::LayoutWidget)
    MEMBER(mName)
    MEMBER(mWidgetTemplate)
    // MEMBER(mWidget)
    MEMBER(mType)
    MEMBER(mDefaultVisibility)
METATABLE_END(Engine::Widgets::LayoutWidget)

SERIALIZETABLE_BEGIN(Engine::Widgets::LayoutWidget)
    FIELD(mName)
    FIELD(mWidgetTemplate)
    FIELD(mType)
    FIELD(mDefaultVisibility)
SERIALIZETABLE_END(Engine::Widgets::LayoutWidget)

namespace Engine {
namespace Widgets {

}
}