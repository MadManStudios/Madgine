#include "../../widgetslib.h"

#include "renderdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN(Engine::Widgets::RenderData)
METATABLE_END(Engine::Widgets::RenderData)

SERIALIZETABLE_BEGIN(Engine::Widgets::RenderData)
SERIALIZETABLE_END(Engine::Widgets::RenderData)

namespace Engine {
namespace Widgets {

    bool RenderData::animationInterval(std::chrono::steady_clock::duration dur, std::chrono::steady_clock::duration on)
    {
        return std::chrono::steady_clock::now().time_since_epoch().count() % dur.count() < on.count();
    }

}
}