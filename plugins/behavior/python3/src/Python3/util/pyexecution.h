#pragma once

#include "Generic/genericresult.h"

#include "Generic/execution/concepts.h"

#include "pyframeptr.h"

#include "Generic/closure.h"

#include "Madgine/behaviorreceiver.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {
        void setupExecution();

        bool stackUnwindable();

        
        struct CodeObject {
            PyObjectPtr mCode;
            PyObjectPtr mGlobals;
            PyObjectPtr mLocals;
        };

        using ExecutionData = std::variant<
            CodeObject,
            PyFramePtr,
            BehaviorError>;


        PyObject *evalFrame(PyThreadState *tstate, _PyInterpreterFrame *frame, int throwExc);
        void evalCode(BehaviorReceiver &receiver, CodeObject code);
        void evalFrame(BehaviorReceiver &receiver, PyFramePtr frame);
        void evalFrames(BehaviorReceiver &receiver, std::vector<PyFramePtr> frames);

        PyObject *suspend(Closure<void(BehaviorReceiver &, std::vector<PyFramePtr>, Log::Log*, std::stop_token)> callback);

        MADGINE_PYTHON3_EXPORT BehaviorError fetchError();

        struct MADGINE_PYTHON3_EXPORT ExecutionState : BehaviorReceiver {
            ExecutionState(ExecutionData data);
            ~ExecutionState();

            void start();

            ExecutionData mData;
        };

        struct ExecutionSender : Execution::base_sender {
            using result_type = BehaviorError;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<ArgumentList>;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, ExecutionSender &&sender, Rec &&rec)
            {
                return VirtualBehaviorState<Rec, ExecutionState> { std::forward<Rec>(rec), std::move(sender.mData) };
            }

            ExecutionData mData;
        };

    }
}
}