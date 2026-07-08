
#include "../../scenelib.h"

#include "entitydescriptor.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "Meta/reflect/metatable_impl.h"

#include "entity.h"
#include "entitycomponentbase.h"

METATABLE_BEGIN(Engine::Scene::Entity::EntityDescriptor)
    CONSTRUCTOR(Engine::Reflect::Variadic<Engine::Reflect::Derived<Engine::Scene::Entity::EntityComponentBase>>)
METATABLE_END(Engine::Scene::Entity::EntityDescriptor)

namespace Engine {
namespace Scene {
    namespace Entity {

        void ComponentDeleter::operator()(EntityComponentBase *component) const
        {
            EntityComponentRegistry::get(mType).destroy(component);
        }

        ComponentEntry::ComponentEntry(const Reflect::ScopePtr &component)
        {
            for (size_t i = 0; i < EntityComponentRegistry::sComponents().size(); ++i) {
                const EntityComponentRegistry::Annotations &annotation = EntityComponentRegistry::get(i);
                if (*annotation.mType == component.mType) {
                    mComponent = { construct(annotation).release(), { i } };
                    return;
                }
            }
            throw 0;
        }

        EntityDescriptor::EntityDescriptor(std::span<const Reflect::ScopePtr> components)
            : mComponents(components.begin(), components.end())
        {
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
