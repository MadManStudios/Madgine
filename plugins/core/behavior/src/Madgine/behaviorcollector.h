#pragma once

#include "Modules/uniquecomponent/uniquecomponent.h"
#include "Modules/uniquecomponent/uniquecomponentdefine.h"

#include "Modules/threading/taskfuture.h"

#include "Generic/opaqueptr.h"

namespace Engine {

struct BehaviorFactoryBase {
    virtual std::vector<std::string_view> names() const = 0;
    virtual UniqueOpaquePtr load(std::string_view name) const = 0;
    virtual Threading::TaskFuture<bool> state(const UniqueOpaquePtr &handle) const = 0;
    virtual void release(UniqueOpaquePtr &ptr) const = 0;
    virtual std::string_view name(const UniqueOpaquePtr &handle) const = 0;
    virtual Behavior create(const UniqueOpaquePtr &handle, const ParameterTuple &args) const = 0;
    virtual Threading::TaskFuture<ParameterTuple> createParameters(const UniqueOpaquePtr &handle) const = 0;
    virtual ParameterTuple createDummyParameters(const UniqueOpaquePtr &handle) const = 0;
    virtual std::vector<ValueTypeDesc> parameterTypes(const UniqueOpaquePtr &handle) const = 0;
    virtual std::vector<ValueTypeDesc> resultTypes(const UniqueOpaquePtr &handle) const = 0;
    virtual std::vector<BindingDescriptor> bindings(const UniqueOpaquePtr &handle) const = 0;
};

struct BehaviorFactoryAnnotation {
    template <typename T>
    BehaviorFactoryAnnotation(type_holder_t<T>)
        : mFactory(&T::sFactory)
    {
    }

    const BehaviorFactoryBase *mFactory;
};
}

DECLARE_NAMED_UNIQUE_COMPONENT(Engine, BehaviorFactory, BehaviorFactoryBase, BehaviorFactoryAnnotation)

namespace Engine {

template <typename T>
struct BehaviorFactory : BehaviorFactoryComponent<T> {
    static T sFactory;
};

template <typename T>
T BehaviorFactory<T>::sFactory;

}

REGISTER_TYPE(Engine::BehaviorFactoryBase)

#define DECLARE_BEHAVIOR_FACTORY(Factory) \
    REGISTER_TYPE(Factory)

#define DEFINE_BEHAVIOR_FACTORY(Name, Factory) \
    NAMED_UNIQUECOMPONENT(Name, Factory)
