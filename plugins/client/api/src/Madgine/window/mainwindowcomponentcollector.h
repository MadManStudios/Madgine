#pragma once

#include "Modules/uniquecomponent/uniquecomponentdefine.h"

#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Meta/keyvalue/virtualscope.h"

#include "Meta/serialize/helper/annotations.h"

DECLARE_NAMED_UNIQUE_COMPONENT(Engine::Window, MainWindowComponent, MainWindowComponentBase,
    Engine::UniqueComponent::Constructor<MainWindow &>,
    Engine::Serialize::TypeAnnotation)

namespace Engine {
namespace Window {

    template <typename T>
    struct MainWindowComponent : Serialize::VirtualData<T, VirtualScope<T, MainWindowComponentComponent<T>>> {
        using Serialize::VirtualData<T, VirtualScope<T, MainWindowComponentComponent<T>>>::VirtualData;

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
