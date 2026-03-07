#include "../debuglib.h"

#include "senderlocation.h"

#include "Meta/keyvalue/valuetype.h"

#include "debugger.h"

namespace Engine {
namespace Debug {

    SenderLocation::SenderLocation(Closure<void(CallableView<void(const Execution::StateDescriptor &)>)> state)
        : mState(std::move(state))
    {
    }

    void SenderLocation::stepInto(Debug::SenderLocation *&location, ContextInfo &context)
    {
        std::unique_lock lock { context.mMutex };
        assert(!location);
        location = this;
    }

    void SenderLocation::stepOut(Debug::SenderLocation *&location, ContextInfo &context)
    {
        std::unique_lock lock { context.mMutex };
        assert(location == this);
        location = nullptr;
    }

    void SenderLocation::visit(CallableView<void(const Execution::StateDescriptor &)> visitor) const
    {
        mState(std::move(visitor));
    }

}
}