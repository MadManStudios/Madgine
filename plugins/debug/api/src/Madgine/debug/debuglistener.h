#pragma once

namespace Engine {
namespace Debug {

	
    struct DebugListener {
        virtual bool wantsPause(const DebugLocation &location, ContinuationType type)
        {
            return false;
        }
        virtual void onSuspend(ContextInfo &context, ContinuationType type) { }
    };


}
}