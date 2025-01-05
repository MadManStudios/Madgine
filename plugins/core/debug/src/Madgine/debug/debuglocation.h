#pragma once

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
        virtual bool wantsPause(ContinuationType type) const = 0;

        void stepInto(ParentLocation *parent);
        void stepOut(ParentLocation *parent);
        template <typename F, typename... Args>
        void yield(F &&callback, std::stop_token st, ContinuationType type, Args &&...args)
        {
            mContext->suspend({ std::forward<F>(callback), type, std::forward<Args>(args)... }, std::move(st));
        }

        template <typename F, typename... Args>
        void pass(F &&callback, std::stop_token st, ContinuationType type, bool forceStop = false, Args &&...args)
        {
            if (forceStop || wantsPause(type)) {
                yield(std::forward<F>(callback), std::move(st), type, std::forward<Args>(args)...);
            } else {
                std::forward<F>(callback)(ContinuationMode::Continue, std::forward<Args>(args)...);
            }
        }
    };


}
}