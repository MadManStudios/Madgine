#include "../behaviorlib.h"

#include "behaviorhandle.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "Meta/reflect/metatable_impl.h"

#include "behaviorsender.h"
#include "behaviorcollector.h"
#include "parametertuple.h"

METATABLE_BEGIN(Engine::Behavior::BehaviorHandle)
    //CONSTRUCTOR()
// MEMBER(mName)
METATABLE_END(Engine::Behavior::BehaviorHandle)

namespace Engine {
namespace Behavior {

    BehaviorHandle::BehaviorHandle(IndexType<uint32_t> index, std::string_view name)
        : mIndex(index)
        , mHandle(BehaviorFactoryRegistry::get(mIndex).mFactory->load(name))
    {
    }

    BehaviorHandle::BehaviorHandle(const BehaviorHandle &other)
        : mIndex(other.mIndex)
        , mHandle(BehaviorFactoryRegistry::get(mIndex).mFactory->load(other.name()))
    {
    }

    BehaviorHandle::~BehaviorHandle()
    {
        reset();
    }

    BehaviorHandle &BehaviorHandle::operator=(const BehaviorHandle &other)
    {
        if (mHandle)
            BehaviorFactoryRegistry::get(mIndex).mFactory->release(mHandle);
        mIndex = other.mIndex;
        mHandle = BehaviorFactoryRegistry::get(mIndex).mFactory->load(other.name());
        return *this;
    }

    BehaviorHandle &BehaviorHandle::operator=(BehaviorHandle &&other) noexcept
    {
        std::swap(mIndex, other.mIndex);
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    void BehaviorHandle::reset()
    {
        if (mHandle)
            BehaviorFactoryRegistry::get(mIndex).mFactory->release(mHandle);
        mIndex.reset();
    }

    Behavior BehaviorHandle::create(const ParameterTuple &args, std::vector<Behavior> behaviors) const
    {
        return BehaviorFactoryRegistry::get(mIndex).mFactory->create(mHandle, args, std::move(behaviors));
    }

    BehaviorSender BehaviorHandle::sender() const
    {
        return { *this };
    }

    Threading::TaskFuture<bool> BehaviorHandle::state() const
    {
        return BehaviorFactoryRegistry::get(mIndex).mFactory->state(mHandle);
    }

    const BehaviorDescriptor &BehaviorHandle::descriptor() const
    {
        return BehaviorFactoryRegistry::get(mIndex).mFactory->descriptor(mHandle);
    }

    std::string_view BehaviorHandle::name() const
    {
        return BehaviorFactoryRegistry::get(mIndex).mFactory->name(mHandle);
    }

    std::string BehaviorHandle::toString() const
    {
        return std::string { BehaviorFactoryRegistry::sComponentName(mIndex) } + "/" + std::string { name() };
    }

    bool BehaviorHandle::fromString(std::string_view s)
    {
        size_t separator = s.find('/');
        if (separator == std::string_view::npos)
            return false;
        std::string_view category = s.substr(0, separator);
        std::string_view name = s.substr(separator + 1);

        auto it = BehaviorFactoryRegistry::sComponentsByName().find(category);

        if (it == BehaviorFactoryRegistry::sComponentsByName().end())
            return false;

        mIndex = it->second;
        mHandle = BehaviorFactoryRegistry::get(mIndex).mFactory->load(name);

        if (!mHandle)
            return false;

        return true;
    }

    BehaviorHandle::operator bool() const
    {
        return static_cast<bool>(mIndex);
    }

}
}
