#include "../debuglib.h"

#include "senderlocation.h"

#include "Meta/keyvalue/valuetype.h"

namespace Engine {
namespace Debug {

    SenderLocation::SenderLocation(Closure<void(CallableView<void(const Execution::StateDescriptor &)>)> state)
        : mState(std::move(state))
    {
    }

    void SenderLocation::stepInto(Debug::ParentLocation *parent)
    {
        mIndex = 0;
        Debug::DebugLocation::stepInto(parent);
    }

    std::string SenderLocation::toString() const
    {
        return "Sender";
    }

    std::map<std::string_view, ValueType> SenderLocation::localVariables() const
    {
        return {};
    }

    bool SenderLocation::wantsPause(Debug::ContinuationType type) const
    {
        return type == Debug::ContinuationType::Error || Debug::DebugLocation::wantsPause(type);
    }

    void SenderLocation::visit(CallableView<void(const Execution::StateDescriptor &)> visitor) const
    {
        mState(std::move(visitor));
    }

}
}