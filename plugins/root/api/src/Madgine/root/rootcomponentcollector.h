#pragma once

#include "Modules/uniquecomponent/uniquecomponent.h"
#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_UNIQUE_COMPONENT(Engine::Core, RootComponent, RootComponentBase, Engine::Plugins::Constructor<Engine::Core::Root &>)

namespace Engine {
namespace Core {

    MADGINE_ROOT_EXPORT RootComponentBase &getRootComponent(size_t i);

    template <typename T>
    struct RootComponent : Reflect::VirtualScope<T, RootComponentComponent<T>> {

        using Reflect::VirtualScope<T, RootComponentComponent<T>>::VirtualScope;

        static T &getSingleton()
        {
            return static_cast<T &>(getRootComponent(Engine::Plugins::component_index<T>()));
        }
    };

    template <typename T>
    struct VirtualRootComponentBase : RootComponentVirtualBase<T> {
        using RootComponentVirtualBase<T>::RootComponentVirtualBase;

        static T &getSingleton()
        {
            return static_cast<T &>(getRootComponent(Engine::Plugins::component_index<T>()));
        }
    };
}
}
