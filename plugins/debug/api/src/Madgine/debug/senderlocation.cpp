#include "../debuglib.h"

#include "senderlocation.h"

#include "Meta/keyvalue/valuetype.h"

namespace Engine {
namespace Debug {

    SenderLocation::SenderLocation(Closure<void(CallableView<void(const Execution::StateDescriptor &)>)> state)
        : mState(std::move(state))
    {
    }

    std::string SenderLocation::toString() const
    {
        std::string result = "Sender";
        bool done = false;
        auto cb = [&](const Execution::StateDescriptor &desc) {
            if (!done) {
                if (std::holds_alternative<Execution::State::SubLocation>(desc)) {
                    if (mChild) {
                        result = mChild->toString();
                        done = true;
                    }
                } else if (std::holds_alternative<Execution::State::BeginBlock>(desc)) {
                    result = std::get<Execution::State::BeginBlock>(desc).mName;
                    done = true;
                }
            }
        };
        visit(CallableView<void(const Execution::StateDescriptor &)> { cb });
        return result;
    }

    std::map<std::string_view, ValueType> SenderLocation::localVariables() const
    {
        return {};
    }

    bool SenderLocation::wantsPause(Debug::ContinuationType type, IndexType<size_t> line) const
    {
        return type == Debug::ContinuationType::Error || (line && getBreakpoint(line)) || Debug::DebugLocation::wantsPause(type, line);
    }

    void SenderLocation::visit(CallableView<void(const Execution::StateDescriptor &)> visitor) const
    {
        mState(std::move(visitor));
    }

    void SenderLocation::setBreakpoint(size_t index, bool set) const
    {
        if (mBreakpoints.size() <= index) {
            mBreakpoints.resize(index + 1);
        }
        mBreakpoints[index] = set;
    }

    bool SenderLocation::getBreakpoint(size_t index) const
    {
        return mBreakpoints.size() > index && mBreakpoints[index];
    }

}
}