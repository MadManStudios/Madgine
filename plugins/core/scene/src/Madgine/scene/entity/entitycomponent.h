#pragma once

#include "entitycomponentbase.h"
#include "entitycomponentcollector.h"

#include "entitycomponentlist.h"

namespace Engine {
namespace Scene {
    namespace Entity {

        template <typename T, typename Base>
        using VirtualEntityComponentImpl = VirtualScope<T, UniqueComponent::VirtualComponentImpl<T, Base>>;

        template <typename T>
        using VirtualEntityComponentBase = EntityComponentVirtualBase<T>;

        template <typename T, typename Base = EntityComponentBase>
        using EntityComponent = EntityComponentComponent<T, Base>;

#define ENTITYCOMPONENTVIRTUALBASE_IMPL(Name, FullType) \
    COMPONENT_NAME(Name, FullType)

#define ENTITYCOMPONENTVIRTUALIMPL_IMPL(Name) \
    VIRTUALUNIQUECOMPONENT(Name)

#define ENTITYCOMPONENT_IMPL(Name, Type) \
    NAMED_UNIQUECOMPONENT(Name, Type)    \
    UNIQUECOMPONENT2(Engine::Scene::Entity::EntityComponentList<Type>, _2)

    }
}
}
