#include "../../../scenelib.h"

#include "material.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

NAMED_UNIQUECOMPONENT(Material, Engine::Scene::Entity::Material);

METATABLE_BEGIN(Engine::Scene::Entity::Material)
    MEMBER(mMaterial)
METATABLE_END(Engine::Scene::Entity::Material)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::Material)
    FIELD(mMaterial)
SERIALIZETABLE_END(Engine::Scene::Entity::Material)

namespace Engine {
namespace Scene {
    namespace Entity {

    }
}
}
