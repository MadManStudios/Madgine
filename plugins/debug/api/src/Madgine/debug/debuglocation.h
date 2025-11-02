#pragma once

#include "continuation.h"

namespace Engine {
namespace Debug {

    struct MADGINE_DEBUGGER_EXPORT ParentLocation {

        DebugLocation *currentLocation() const;

        DebugLocation *mChild = nullptr;
        ContextInfo *mContext = nullptr;
    };

    struct MADGINE_DEBUGGER_EXPORT DebugLocation : ParentLocation {
        virtual ~DebugLocation() = default;
        virtual std::string toString() const = 0;
        virtual std::map<std::string_view, ValueType> localVariables() const = 0;
        virtual bool wantsPause(ContinuationType type, IndexType<size_t> line) const = 0;

        void stepInto(ParentLocation *parent);
        void stepOut(ParentLocation *parent);
        template <typename F, typename... Args>
        void yield(F &&callback, Continuation &outContinuation, ContinuationType type, Execution::StopToken st, Args &&...args)
        {
            yieldImpl({ std::forward<F>(callback), type, std::forward<Args>(args)... }, outContinuation, std::move(st));
        }

        template <typename F, typename... Args>
        void pass(F &&callback, Continuation &outContinuation, ContinuationType type, Execution::StopToken st, IndexType<size_t> line = {}, Args &&...args)
        {
            if (wantsPause(type, line)) {
                yield(std::forward<F>(callback), outContinuation, type, std::move(st), std::forward<Args>(args)...);
            } else {
                std::forward<F>(callback)(ContinuationMode::Continue, std::forward<Args>(args)...);
            }
        }

    private:
        void yieldImpl(Continuation cont, Continuation &outContinuation, Execution::StopToken st);
    };

}
}