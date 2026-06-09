#pragma once

#include "Generic/execution/stoppable.h"

#include "Meta/reflect/sender.h"

#include "Madgine/behavior/behaviorreceiver.h"
#include "Madgine/debug/debuggablesender.h"

#include "pyobjectptr.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        MADGINE_PYTHON3_EXPORT extern PyTypeObject PySenderType;
        MADGINE_PYTHON3_EXPORT extern PyTypeObject PySenderStateType;

        struct PySender {
            PyObject_HEAD
                Reflect::Sender mSender;
        };

        struct SenderState;

        struct SenderReceiver {

            void set_value(const Reflect::ArgumentList &values);
            void set_error(Reflect::Error error);
            void set_done();

            template <typename CPO, typename... Args>
                requires(is_tag_invocable_v<CPO, BehaviorReceiver &, Args...>)
            friend auto tag_invoke(CPO f, SenderReceiver &state, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, BehaviorReceiver &, Args...>)
                -> tag_invoke_result_t<CPO, BehaviorReceiver &, Args...>;

            SenderState &mState;
        };

        struct SenderState {

            ~SenderState();
            void resume();

            using Inner = Execution::connect_result_t<Execution::with_debug_location_t::sender<Execution::stoppable_t::sender<Reflect::Sender>>, SenderReceiver>;

            BehaviorReceiver &mReceiver;
            std::atomic_flag mFlag;
            PyObjectPtr mCoroutine;
            PyObjectPtr mResult;
            ManualLifetime<Inner> mInnerState;

            Debug::SenderLocation *mChild = nullptr;
        };

        struct PySenderState {
            PyObject_HEAD
                SenderState mState;
        };

        template <typename CPO, typename... Args>
            requires(is_tag_invocable_v<CPO, BehaviorReceiver &, Args...>)
        auto tag_invoke(CPO f, SenderReceiver &state, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, BehaviorReceiver &, Args...>)
            -> tag_invoke_result_t<CPO, BehaviorReceiver &, Args...>
        {
            return tag_invoke(f, state.mState.mReceiver, std::forward<Args>(args)...);
        }

    }
}
}
