#pragma once

#include "Generic/callable_view.h"
#include "Generic/closure.h"
#include "Generic/execution/statedescriptor.h"
#include "debuglocation.h"

namespace Engine {
namespace Debug {

    struct MADGINE_DEBUGGER_EXPORT SenderLocation : DebugLocation {
        SenderLocation(Closure<void(CallableView<void(const Execution::StateDescriptor &)>)> state);

        void stepInto(DebugLocation &child) override;
        void stepOut(DebugLocation &child) override;

        std::string toString() const override;
        std::map<std::string_view, ValueType> localVariables() const override;
        bool wantsPause(Debug::ContinuationType type, IndexType<size_t> line) const override;

        void visit(CallableView<void(const Execution::StateDescriptor &)> visitor) const;

        void setBreakpoint(size_t index, bool set) const;
        bool getBreakpoint(size_t index) const;

        Closure<void(CallableView<void(const Execution::StateDescriptor &)>)> mState;
        mutable std::vector<bool> mBreakpoints;
    };

}
}