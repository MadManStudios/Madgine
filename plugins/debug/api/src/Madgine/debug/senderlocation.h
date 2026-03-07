#pragma once

#include "Generic/callable_view.h"
#include "Generic/closure.h"
#include "Generic/execution/statedescriptor.h"

#include "debuglocation.h"

namespace Engine {
namespace Debug {

    struct MADGINE_DEBUGGER_EXPORT SenderLocation {
        SenderLocation(Closure<void(CallableView<void(const Execution::StateDescriptor &)>)> state);

        void stepInto(Debug::SenderLocation *&location, ContextInfo &context);
        void stepOut(Debug::SenderLocation *&location, ContextInfo &context);

        void visit(CallableView<void(const Execution::StateDescriptor &)> visitor) const;

        Closure<void(CallableView<void(const Execution::StateDescriptor &)>)> mState;    
    };

}
}