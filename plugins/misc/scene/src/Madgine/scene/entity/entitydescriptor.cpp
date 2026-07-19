
#include "../../scenelib.h"

#include "entitydescriptor.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Meta/type/storageops_impl.h"

#include "entity.h"
#include "entitycomponentbase.h"

METATABLE_BEGIN(Engine::Scene::Entity::EntityDescriptor)    
METATABLE_END(Engine::Scene::Entity::EntityDescriptor)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::EntityDescriptor)
SERIALIZETABLE_END(Engine::Scene::Entity::EntityDescriptor)

STORAGEOPS_BEGIN(Engine::Scene::Entity::EntityDescriptor)
CONSTRUCTOR(Engine::Type::Variadic<Engine::Type::Derived<Engine::Scene::Entity::EntityComponentBase>>)
STORAGEOPS_END(Engine::Scene::Entity::EntityDescriptor)


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
