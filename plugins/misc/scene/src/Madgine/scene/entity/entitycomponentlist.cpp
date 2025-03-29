#include "../../scenelib.h"

#include "entitycomponentlist.h"

#include "entity.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        void relocateEntityComponent(Entity* entity, EntityComponentHandle<EntityComponentBase> index) {
            entity->relocateComponent(index);
        }

    }
}
}
