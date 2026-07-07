
#include "../../scenelib.h"

#include "entitydescriptor.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "entity.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        void ComponentDeleter::operator()(EntityComponentBase *component) const
        {
            EntityComponentRegistry::get(mType).destroy(component);
        }

        void EntityDescriptor::apply(Entity &entity) const
        {
            for (const ComponentEntry &entry : mComponents) {
                entity.copyComponent(entry.mComponent.get_deleter().mType, *entry.mComponent);
            }
        }

    }
}
}
