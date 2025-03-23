#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"
#include "Modules/uniquecomponent/uniquecomponent.h"

DECLARE_UNIQUE_COMPONENT(Engine::Root, RootComponent, RootComponentBase, Engine::UniqueComponent::Constructor<Engine::Root::Root &>)

namespace Engine {
namespace Root {

    MADGINE_ROOT_EXPORT RootComponentBase &getRootComponent(size_t i);

    template <typename T>
    struct RootComponent : VirtualScope<T, RootComponentComponent<T>> {

        using VirtualScope<T, RootComponentComponent<T>>::VirtualScope;

        static T& getSingleton() {
            return static_cast<T&>(getRootComponent(Engine::UniqueComponent::component_index<T>()));
        }
    };

    template <typename T>
    struct VirtualRootComponentBase : RootComponentVirtualBase<T> {
        using RootComponentVirtualBase<T>::RootComponentVirtualBase;

        static T &getSingleton()
        {
            return static_cast<T &>(getRootComponent(Engine::UniqueComponent::component_index<T>()));
        }
    };
}
}
