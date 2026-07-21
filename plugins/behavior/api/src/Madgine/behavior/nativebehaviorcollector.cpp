#include "../behaviorlib.h"

#include "nativebehaviorcollector.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"
#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "behavior.h"

DEFINE_UNIQUE_COMPONENT(Engine::Behavior, NativeBehavior)

BEHAVIOR_FACTORY(Native, Engine::Behavior::NativeBehaviorFactory)

namespace Engine {
namespace Behavior {

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
        const NativeBehaviorInfo *info = handle.as<const NativeBehaviorInfo *>();
        return info->name();
    }

    Behavior NativeBehaviorFactory::create(const UniqueOpaquePtr &handle, const Reflect::ArgumentList &args, std::vector<Behavior> behaviors) const
    {
        const NativeBehaviorInfo *info = handle.as<const NativeBehaviorInfo *>();
        return info->create(args, std::move(behaviors));
    }

    ParameterTuple NativeBehaviorFactory::createParameters(const UniqueOpaquePtr &handle) const
    {
        const NativeBehaviorInfo *info = handle.as<const NativeBehaviorInfo *>();
        return info->createParameters();
    }

    const BehaviorDescriptor &NativeBehaviorFactory::descriptor(const UniqueOpaquePtr &handle) const
    {
        const NativeBehaviorInfo *info = handle.as<const NativeBehaviorInfo *>();
        return info->descriptor();
    }

}
}
