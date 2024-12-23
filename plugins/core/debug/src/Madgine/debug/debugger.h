#pragma once

#include "Generic/callable_view.h"
#include "Generic/closure.h"
#include "Generic/execution/statedescriptor.h"
#include "Generic/execution/stop_callback.h"
#include "Generic/execution/virtualsender.h"
#include "Generic/forward_capture.h"
#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

namespace Engine {
namespace Debug {

    enum class ContinuationControl {
        Resume,
        Step,
        Pause,
        Stop
    };

    enum class ContinuationMode {
        Continue,
        Abort
    };

    enum class ContinuationType {
        Flow,
        Return,
        Error,
        Cancelled
    };

    struct Continuation {
    private:
        struct Base {
            virtual ~Base() = default;

            virtual void call(ContinuationMode mode) = 0;

            virtual void visitArguments(std::ostream &) = 0;
        };

        template <typename F, typename... Args>
        struct Impl : Base {
            Impl(F &&callback, Args &&...args)
                : mCallback(std::forward<F>(callback))
                , mArgs { std::forward<Args>(args)... }
            {
            }

            virtual void call(ContinuationMode mode) override
            {
                TupleUnpacker::invokeExpand(std::forward<F>(mCallback), mode, std::move(mArgs));
            }

            virtual void visitArguments(std::ostream &out) override
            {
                StringUtil::StreamJoiner joiner { out, "\n" };
                TupleUnpacker::forEach(mArgs, [&](auto &v) {
                    if constexpr (requires { out << v; }) {
                        joiner.next() << v;
                    } else {
                        joiner.next() << typeid(v).name();
                    }
                });
            }

            F mCallback;
            std::tuple<Args...> mArgs;
        };

    public:
        Continuation() = default;

        template <typename F, typename... Args>
        Continuation(F &&callback, ContinuationType type, Args &&...args)
            : mImpl(std::make_unique<Impl<F, Args...>>(std::forward<F>(callback), std::forward<Args>(args)...))
            , mType(type)
        {
        }

        explicit operator bool() const
        {
            return static_cast<bool>(mImpl);
        }

        ContinuationType type() const
        {
            return mType;
        }

        void operator()(ContinuationMode mode)
        {
            mImpl->call(mode);
            mImpl.reset();
        }

        void visitArguments(std::ostream &out) const
        {
            mImpl->visitArguments(out);
        }

    private:
        std::unique_ptr<Base> mImpl;
        ContinuationType mType;
    };

    struct MADGINE_DEBUGGER_EXPORT ParentLocation {

        DebugLocation *currentLocation() const;

        DebugLocation *mChild = nullptr;
        ContextInfo *mContext = nullptr;
    };

    struct MADGINE_DEBUGGER_EXPORT ContextInfo : ParentLocation {
        ContextInfo()
            : ParentLocation { nullptr, this }
            , mStopCallback(finally_cb { this })
        {
        }

        void suspend(Continuation callback, std::stop_token st);
        void continueExecution(ContinuationMode mode);

        void control(ContinuationControl control);
        void resume();
        void step();
        void pause();
        void stop();

        bool alive() const;
        bool isPaused() const;

        std::string getArguments() const;
        ContinuationType continuationType() const;

        struct stop_cb {
            ContextInfo *mContext;
            bool operator()() const;
        };

        struct finally_cb {
            ContextInfo *mContext;
            void operator()(ContinuationMode mode) const;
            void operator()(Execution::cancelled_t) const;
        };

        mutable std::mutex mMutex;

        friend struct Debugger;

    private:
        Continuation mCallback;
        Execution::stop_callback<stop_cb, finally_cb> mStopCallback;
        std::atomic<int> mPaused = 0;
        bool mPauseRequested = false;
        bool mStopRequested = false;
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

    struct DebugListener {
        virtual bool wantsPause(const DebugLocation *location, ContinuationType type)
        {
            return false;
        }
        virtual void onSuspend(ContextInfo &context, ContinuationType type) { }
    };

    struct MADGINE_DEBUGGER_EXPORT Debugger : Root::RootComponent<Debugger> {

        friend struct ContextInfo;

        Debugger(const Debugger &) = delete;

        using Root::RootComponent<Debugger>::RootComponent;

        virtual std::string_view key() const override;

        std::deque<ContextInfo> &infos();
        ContextInfo &createContext();

        void addListener(DebugListener *listener);
        void removeListener(DebugListener *listener);

        bool wantsPause(const DebugLocation *location, ContinuationType type);

    private:
        std::deque<ContextInfo> mContexts;
        std::vector<DebugListener *> mListeners;
    };

}
}

REGISTER_TYPE(Engine::Debug::Debugger)