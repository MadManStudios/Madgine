#include "../../scenelib.h"

#include "entitycomponentcollector.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "entitycomponentlistbase.h"

DEFINE_UNIQUE_COMPONENT(Engine::Scene::Entity, EntityComponent)

namespace Engine {
namespace Scene {
    namespace Entity {

        std::unique_ptr<EntityComponentListBase> tag_invoke(construct_t, const EntityComponentListAnnotation &object, EntityComponentListTag)
        {
            return object.mCtor();
        }

    }
}
}