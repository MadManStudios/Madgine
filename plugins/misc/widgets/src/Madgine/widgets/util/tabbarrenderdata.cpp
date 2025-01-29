#include "../../widgetslib.h"

#include "tabbarrenderdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN_BASE(Engine::Widgets::TabBarRenderData, Engine::Widgets::RenderData)
METATABLE_END(Engine::Widgets::TabBarRenderData)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::TabBarRenderData, Engine::Widgets::RenderData)
SERIALIZETABLE_END(Engine::Widgets::TabBarRenderData)

namespace Engine {
namespace Widgets {

}
}
