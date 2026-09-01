#include "../../../scenelib.h"

#include "pointlight.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

NAMED_UNIQUECOMPONENT(PointLight, Engine::Scene::Entity::PointLight);

METATABLE_BEGIN_BASE(Engine::Scene::Entity::PointLight, Engine::Scene::Entity::EntityComponentBase)
    MEMBER(mRange)
    MEMBER(mColor)
METATABLE_END(Engine::Scene::Entity::PointLight)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::PointLight)
    FIELD(mRange)
    FIELD(mColor)
SERIALIZETABLE_END(Engine::Scene::Entity::PointLight)

namespace Engine {
namespace Scene {
    namespace Entity {

    }
}
}
