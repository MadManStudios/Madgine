#pragma once

#include "Generic/execution/stoppable.h"

#include "Meta/reflect/result.h"

#include "Madgine/debug/debuggablesender.h"

#include "pyobjectptr.h"
#include "python3lock.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        MADGINE_PYTHON3_EXPORT void handleExecutionObject(PyObject *obj);
        MADGINE_PYTHON3_EXPORT void handleExecutionError(Reflect::Error error);

        struct ExecutionState {
            BehaviorReceiver *mReceiver = nullptr;
            Platform::Log::Log *mLog = nullptr;
        };
        ExecutionState &executionState();

        bool lock(BehaviorReceiver * = nullptr, Platform::Log::Log * = nullptr);
        ExecutionState unlock();

        MADGINE_PYTHON3_EXPORT extern PyTypeObject PyDebugLineType;
        MADGINE_PYTHON3_EXPORT extern PyTypeObject PyStateType;

        struct DebugLine : Execution::StopCallback {

            void stopRequested() override;

            size_t mLineNr;
            Debug::Continuation mContinuation;
        };

        struct PyDebugLine {
            PyObject_HEAD
                DebugLine mLine;
        };

        MADGINE_PYTHON3_EXPORT void resumeCoroutine(PyObjectPtr coro, PyObjectPtr value);

        extern PyMethodDef PyStateMethods[];

        struct PyStateBase {

            ~PyStateBase();
            void resume();

            std::atomic_flag mFlag;
            PyObjectPtr mCoroutine;
            PyObjectPtr mResult;

            Debug::SenderLocation *mChild = nullptr;

            void (*mDestruct)(PyStateBase &) = nullptr;
        };

        struct PyStateHelper {
            PyObject_HEAD
                PyStateBase mState;
        };

        struct PyReceiver {

            void set_value(const Reflect::ArgumentList &values);
            void set_error(Reflect::Error error);
            void set_done();

            template <typename CPO, typename... Args>
                requires(is_tag_invocable_v<CPO, BehaviorReceiver &, Args...>)
            friend auto tag_invoke(CPO f, PyReceiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, BehaviorReceiver &, Args...>)
                -> tag_invoke_result_t<CPO, BehaviorReceiver &, Args...>
            {
                return tag_invoke(f, rec.mReceiver, std::forward<Args>(args)...);
            }

            PyStateBase &mState;
            BehaviorReceiver &mReceiver;
        };

        template <typename Sender>
        struct PyState : PyStateBase {
            using Inner = Execution::connect_result_t<Execution::with_debug_location_t::sender<Execution::stoppable_t::sender<Sender>>, PyReceiver>;
            ManualLifetime<Inner> mInnerState;
        };

        template <typename Sender>
        struct PyStateWrapper {
            PyObject_HEAD
                PyState<Sender>
                    mState;
        };

        PyObject *PyState_Alloc(size_t size);

        template <typename Sender>
        PyObject *PyAwait(Sender &&sender)
        {
            PyObject *obj = PyState_Alloc(sizeof(PyStateWrapper<Sender>));
            Python3Suspend suspend;
            PyState<Sender> *state = &reinterpret_cast<PyStateWrapper<Sender> *>(obj)->mState;
            assert(suspend.receiver());
            new (state) PyState<Sender>;

            construct(state->mInnerState, DelayedConstruct { [&]() { return Execution::connect(std::forward<Sender>(sender) | Execution::stoppable | Execution::with_debug_location(state->mChild), PyReceiver { *state, *suspend.receiver() }); } });
            state->mDestruct = [](PyStateBase &state) { destruct(static_cast<PyState<Sender> &>(state).mInnerState); };
            state->mInnerState->start();

            return obj;
        }

    }
}
}