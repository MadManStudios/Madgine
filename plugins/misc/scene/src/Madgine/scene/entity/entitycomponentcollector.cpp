#include "../../scenelib.h"

#include "entitycomponentcollector.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

DEFINE_UNIQUE_COMPONENT(Engine::Scene::Entity, EntityComponent)
DEFINE_UNIQUE_COMPONENT(Engine::Scene::Entity, EntityComponentList)

namespace Engine {
namespace Scene {
    namespace Entity {

        /* std::map<std::string_view, IndexRef> &sComponentsByName()
        {
            static std::map<std::string_view, IndexRef> dummy;
            return dummy;
        }*/

    }
}
}