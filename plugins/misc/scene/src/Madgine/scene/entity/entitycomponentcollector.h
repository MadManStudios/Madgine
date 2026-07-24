#pragma once

#include "Meta/reflect/helper/annotations.h"

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        struct EntityComponentListTag {

        };

        struct EntityComponentListAnnotation {
            template <typename T, typename ActualType>
            EntityComponentListAnnotation(type_holder_t<T> t, type_holder_t<ActualType> at)
                : mCtor([]() -> std::unique_ptr<EntityComponentListBase> {
                    return std::make_unique<EntityComponentList<ActualType>>();
                })
            {
            }

            friend std::unique_ptr<EntityComponentListBase> tag_invoke(construct_t, const EntityComponentListAnnotation &object, EntityComponentListTag);

            std::unique_ptr<EntityComponentListBase> (*mCtor)();
        };

    }
}
}

DECLARE_NAMED_UNIQUE_COMPONENT(Engine::Scene::Entity, EntityComponent, EntityComponentBase, Engine::Scene::Entity::EntityComponentListAnnotation, Engine::Reflect::TypeAnnotation, Engine::Plugins::Constructor<>, Engine::Plugins::CopyConstructor, Engine::Plugins::Destructor<>, Engine::Plugins::Copying<>)
