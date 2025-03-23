#pragma once

#include "Generic/callable_view.h"
#include "debuglocation.h"
#include "Generic/execution/statedescriptor.h"

namespace Engine {
namespace Debug {

	
    struct MADGINE_DEBUGGER_EXPORT SenderLocation : Debug::DebugLocation {
        SenderLocation(Closure<void(CallableView<void(const Execution::StateDescriptor &)>)> state);

        void stepInto(Debug::ParentLocation *parent);

        std::string toString() const override;
        std::map<std::string_view, ValueType> localVariables() const override;
        bool wantsPause(Debug::ContinuationType type) const override;

        void visit(CallableView<void(const Execution::StateDescriptor &)> visitor) const;

        Closure<void(CallableView<void(const Execution::StateDescriptor &)>)> mState;
        size_t mIndex = 0;
    };

}
}