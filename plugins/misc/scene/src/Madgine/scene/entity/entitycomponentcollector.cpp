#include "../../scenelib.h"

#include "entitycomponentcollector.h"

#include "entitycomponentlistbase.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

DEFINE_UNIQUE_COMPONENT(Engine::Scene::Entity, EntityComponent)

namespace Engine {
namespace Scene {
    namespace Entity {       

        std::unique_ptr<EntityComponentListBase> tag_invoke(construct_t, const EntityComponentListAnnotation &object)
        {
            return object.mCtor();
        }

    }
}
}