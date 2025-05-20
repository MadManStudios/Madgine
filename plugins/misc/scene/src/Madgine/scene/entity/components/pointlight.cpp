#include "../../../scenelib.h"

#include "pointlight.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"


NAMED_UNIQUECOMPONENT(PointLight, Engine::Scene::Entity::PointLight);

METATABLE_BEGIN(Engine::Scene::Entity::PointLight)
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
