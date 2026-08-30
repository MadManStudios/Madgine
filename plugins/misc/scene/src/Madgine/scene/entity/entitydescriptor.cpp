
#include "../../scenelib.h"

#include "entitydescriptor.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"
#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Meta/type/storageops_impl.h"

#include "entity.h"
#include "entitycomponentbase.h"

namespace Engine {

static constexpr auto componentBuilder()
{
    std::array<Reflect::Accessor, 32> accessors;

    return accessors;
}

static void componentInit(std::array<Reflect::Accessor, 32> &accessors)
{
#if ENABLE_PLUGINS
    Scene::Entity::EntityComponentCollector::addInitializer([&]() {
#endif
        size_t i = 0;
        for (const auto &[name, index] : Scene::Entity::EntityComponentRegistry::sComponentsByName()) {
            accessors[i] = { name.data(),
                [](const Reflect::Accessor *self, const Reflect::Value &desc) {
                    uint32_t index = Scene::Entity::EntityComponentRegistry::sComponentsByName().at(self->mName);
                    bool found = false;
                    Reflect::Result result = invoke_member([&](Scene::Entity::EntityDescriptor &desc) { found = desc.hasComponent(index); }, {}, desc);
                    return !result && found;
                },
                [](const Reflect::Accessor *self, Reflect::Value &ret, const Reflect::Value &desc, Reflect::ContextPtr context) -> Reflect::Result {
                    uint32_t index = Scene::Entity::EntityComponentRegistry::sComponentsByName().at(self->mName);
                    return invoke_member(ret, dynamic_scope_cast(*Scene::Entity::EntityComponentRegistry::get(index).mType, [=](Scene::Entity::EntityDescriptor & desc) { return desc.getComponent(index); }), context, desc);
                },
                nullptr,
                Reflect::ExtendedType {
                    Reflect::ExtendedTypeIndex { Reflect::TypeEnum::ScopeValue }, Scene::Entity::EntityComponentRegistry::get(index).mType } };
            i++;
        }
#if ENABLE_PLUGINS
    });
#endif
}

}

METATABLE_BEGIN(Engine::Scene::Entity::EntityDescriptor)
    STORAGE_BEGIN(Engine::Scene::Entity::EntityDescriptor, Engine::Scene::Entity::EntityDescriptor)
        CONSTRUCTOR(Engine::Type::Variadic<Engine::Type::Derived<Engine::Scene::Entity::EntityComponentBase>>)
    STORAGE_END(Engine::Scene::Entity::EntityDescriptor)
METATABLE_DYNAMIC_END(componentBuilder, componentInit, Engine::Scene::Entity::EntityDescriptor)

SERIALIZETABLE_BEGIN(Engine::Scene::Entity::EntityDescriptor)
SERIALIZETABLE_END(Engine::Scene::Entity::EntityDescriptor)



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
                    mComponent = { construct(annotation, *static_cast<const EntityComponentBase*>(component.mScope)).release(), { i } };
                    return;
                }
            }
            throw 0;
        }

        ComponentEntry::ComponentEntry(const ComponentEntry &other)
            : mComponent { construct(EntityComponentRegistry::get(other.mComponent.get_deleter().mType), *other.mComponent).release(), other.mComponent.get_deleter() }
        {
        }

        ComponentEntry &ComponentEntry::operator=(const ComponentEntry &other)
        {
            mComponent = { construct(EntityComponentRegistry::get(other.mComponent.get_deleter().mType), *other.mComponent).release(), other.mComponent.get_deleter() };
            return *this;
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

        bool EntityDescriptor::hasComponent(size_t index) const
        {
            return std::ranges::contains(mComponents, index, [](const auto &ptr) { return ptr.mComponent.get_deleter().mType; });
        }

        EntityComponentBase *EntityDescriptor::getComponent(size_t index) const
        {
            auto it = std::ranges::find(mComponents, index, [](const auto &ptr) { return ptr.mComponent.get_deleter().mType; });
            return it == mComponents.end() ? nullptr : it->mComponent.get();
        }

    }
}
}
