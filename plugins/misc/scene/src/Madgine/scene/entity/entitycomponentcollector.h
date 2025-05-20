#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct EntityComponentListAnnotation {
            template <typename T, typename ActualType>
            EntityComponentListAnnotation(type_holder_t<T> t, type_holder_t<ActualType> at)
                : mCtor([]() -> std::unique_ptr<EntityComponentListBase> {
                    return std::make_unique<EntityComponentList<ActualType>>();
                })
            {
            }

            friend std::unique_ptr<EntityComponentListBase> tag_invoke(construct_t, const EntityComponentListAnnotation &object);

            std::unique_ptr<EntityComponentListBase> (*mCtor)();
        };

    }
}
}

DECLARE_NAMED_UNIQUE_COMPONENT(Engine::Scene::Entity, EntityComponent, EntityComponentBase, Engine::UniqueComponent::Constructor<Engine::Scene::Entity::Entity *>, Engine::Scene::Entity::EntityComponentListAnnotation)

namespace Engine {
namespace Scene {
    namespace Entity {

        // MADGINE_SCENE_EXPORT std::map<std::string_view, IndexRef> &sComponentsByName();

    }
}
}