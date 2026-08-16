#pragma once

#include "entitycomponentcollector.h"

#include "Modules/uniquecomponent/component_index.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct MADGINE_SCENE_EXPORT ComponentDeleter {
            size_t mType;

            void operator()(EntityComponentBase *component) const;
        };

        struct ComponentEntry {
            template <Concepts::DecayedNoneOf<ComponentEntry> ComponentType>
                requires std::derived_from<ComponentType ,EntityComponentBase>
            ComponentEntry(ComponentType &&component)
                : mComponent(new ComponentType(std::forward<ComponentType>(component)), { Plugins::component_index<ComponentType>() })
            {
            }            

            ComponentEntry(const Reflect::ScopePtr &component);

            ComponentEntry(const ComponentEntry &other);
            ComponentEntry(ComponentEntry &&) = default;

            ComponentEntry &operator=(const ComponentEntry &other);
            ComponentEntry &operator=(ComponentEntry &&) = default;

            std::unique_ptr<EntityComponentBase, ComponentDeleter> mComponent;
        };

        struct EntityDescriptor {
            template <std::derived_from<EntityComponentBase>... Components>
            EntityDescriptor(Components &&...components)
            {
                (mComponents.emplace_back(std::forward<Components>(components)), ...);
            }           

            EntityDescriptor(std::span<const Reflect::ScopePtr> components);

            bool hasComponent(size_t index) const;
            EntityComponentBase *getComponent(size_t index) const;

            void apply(Entity &entity) const;

            std::vector<ComponentEntry> mComponents;
        };

    }
}
}