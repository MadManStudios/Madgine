#pragma once

#include "Meta/reflect/virtualscope.h"
#include "Meta/serialize/helper/annotations.h"
#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

DECLARE_NAMED_UNIQUE_COMPONENT(Engine::Core, MainWindowComponent, MainWindowComponentBase,
    Engine::Plugins::Constructor<MainWindow &>,
    Engine::Serialize::TypeAnnotation)

namespace Engine {
namespace Core {

    template <typename T>
    struct MainWindowComponent : Serialize::VirtualData<T, Reflect::VirtualScope<T, MainWindowComponentComponent<T>>> {
        using Serialize::VirtualData<T, Reflect::VirtualScope<T, MainWindowComponentComponent<T>>>::VirtualData;

        virtual std::string_view key() const override final
        {
            return T::componentName();
        }

        virtual std::string_view name() const override final
        {
            return T::componentName();
        }
    };

    template <typename T>
    struct MainWindowVirtualBase : Serialize::VirtualData<T, MainWindowComponentVirtualBase<T>> {
        using Serialize::VirtualData<T, MainWindowComponentVirtualBase<T>>::VirtualData;

        virtual std::string_view key() const override final
        {
            return T::componentName();
        }

        virtual std::string_view name() const override final
        {
            return T::componentName();
        }
    };

    template <typename T, typename Base>
    using MainWindowVirtualImpl = MainWindowComponentVirtualImpl<T, Base>;

}
}
