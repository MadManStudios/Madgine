#pragma once

#include "Generic/opaqueptr.h"

#include "Modules/threading/taskfuture.h"
#include "Modules/uniquecomponent/uniquecomponent.h"
#include "Modules/uniquecomponent/uniquecomponentdefine.h"

namespace Engine {
namespace Behavior {

    struct BehaviorFactoryBase {
        virtual std::vector<std::string_view> names() const = 0;
        virtual UniqueOpaquePtr load(std::string_view name) const = 0;
        virtual Threading::TaskFuture<bool> state(const UniqueOpaquePtr &handle) const = 0;
        virtual void release(UniqueOpaquePtr &ptr) const = 0;
        virtual std::string_view name(const UniqueOpaquePtr &handle) const = 0;
        virtual Behavior create(const UniqueOpaquePtr &handle, const ParameterTuple &args, std::vector<Behavior> behaviors) const = 0;
        virtual ParameterTuple createParameters(const UniqueOpaquePtr &handle) const = 0;
        virtual std::vector<ExtendedValueTypeDesc> parameterTypes(const UniqueOpaquePtr &handle) const = 0;
        virtual std::vector<ExtendedValueTypeDesc> resultTypes(const UniqueOpaquePtr &handle) const = 0;
        virtual std::vector<NamedDescriptor> namedInputs(const UniqueOpaquePtr &handle) const = 0;
        virtual size_t subBehaviorCount(const UniqueOpaquePtr &handle) const = 0;
    };

    struct BehaviorFactoryAnnotation {
        template <typename T, typename ActualType>
        BehaviorFactoryAnnotation(type_holder_t<T>, type_holder_t<ActualType>)
            : mFactory(&T::sFactory)
        {
        }

        const BehaviorFactoryBase *mFactory;
    };

}
}

DECLARE_NAMED_UNIQUE_COMPONENT(Engine::Behavior, BehaviorFactory, BehaviorFactoryBase, BehaviorFactoryAnnotation)

namespace Engine {
namespace Behavior {

    template <typename T>
    struct BehaviorFactory : BehaviorFactoryComponent<T> {
        static T sFactory;
    };

    template <typename T>
    T BehaviorFactory<T>::sFactory;

}
}

#define BEHAVIOR_FACTORY(Name, Factory) \
    NAMED_UNIQUECOMPONENT(Name, Factory)
