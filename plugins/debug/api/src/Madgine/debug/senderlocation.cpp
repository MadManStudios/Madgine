#include "../debuglib.h"

#include "senderlocation.h"

#include "debugger.h"

namespace Engine {
namespace Debug {

    SenderLocation::SenderLocation(Closure<void(CB)> state)
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

    void SenderLocation::visit(CB visitor) const
    {
        mState(std::move(visitor));
    }

}
}