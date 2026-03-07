#pragma once

namespace Engine {
namespace Debug {

    struct DebugListener {
        virtual bool wantsPause(ContextInfo &context, TypedPtr location, ContinuationType type, IndexType<size_t> line)
        {
            return false;
        }
        virtual void onSuspend(ContextInfo &context, TypedPtr location, ContinuationType type) { }
    };

}
}