#include "behaviorlib.h"

#include "nativebehaviorcollector.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"
#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "behavior.h"

DEFINE_UNIQUE_COMPONENT(Engine, NativeBehavior)

DEFINE_BEHAVIOR_FACTORY(Native, Engine::NativeBehaviorFactory)

namespace Engine {

std::vector<std::string_view> NativeBehaviorFactory::names() const
{
    const auto &names = kvKeys(NativeBehaviorRegistry::sComponentsByName());
    return std::vector<std::string_view> { names.begin(), names.end() };
}

UniqueOpaquePtr NativeBehaviorFactory::load(std::string_view name) const
{
    UniqueOpaquePtr ptr;

    auto it = NativeBehaviorRegistry::sComponentsByName().find(name);

    if (it != NativeBehaviorRegistry::sComponentsByName().end())
        ptr.setupAs<const NativeBehaviorInfo *>() = NativeBehaviorRegistry::get(it->second).mInfo;

    return ptr;
}

Threading::TaskFuture<bool> NativeBehaviorFactory::state(const UniqueOpaquePtr &handle) const
{
    return true;
}

void NativeBehaviorFactory::release(UniqueOpaquePtr &ptr) const
{
    ptr.release<const NativeBehaviorInfo *>();
}

std::string_view NativeBehaviorFactory::name(const UniqueOpaquePtr &handle) const
{
    const NativeBehaviorInfo *info = handle;
    return info->name();
}

Behavior NativeBehaviorFactory::create(const UniqueOpaquePtr &handle, const ParameterTuple &args) const
{
    const NativeBehaviorInfo *info = handle;
    return info->create(args);
}

Threading::TaskFuture<ParameterTuple> NativeBehaviorFactory::createParameters(const UniqueOpaquePtr &handle) const
{
    const NativeBehaviorInfo *info = handle;
    return info->createParameters();
}

ParameterTuple NativeBehaviorFactory::createDummyParameters(const UniqueOpaquePtr &handle) const
{
    const NativeBehaviorInfo *info = handle;
    return info->createParameters();
}

std::vector<ValueTypeDesc> NativeBehaviorFactory::parameterTypes(const UniqueOpaquePtr &handle) const
{
    const NativeBehaviorInfo *info = handle;
    auto types = info->parameterTypes();
    return { types.begin(), types.end() };
}

std::vector<ValueTypeDesc> NativeBehaviorFactory::resultTypes(const UniqueOpaquePtr &handle) const
{
    const NativeBehaviorInfo *info = handle;
    auto types = info->resultTypes();
    return { types.begin(), types.end() };
}

std::vector<BindingDescriptor> NativeBehaviorFactory::bindings(const UniqueOpaquePtr &handle) const
{
    const NativeBehaviorInfo *info = handle;
    auto bindings = info->bindings();
    return { bindings.begin(), bindings.end() };
}

}
