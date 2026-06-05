#include "../nodegraphlib.h"

#include "nodeexecution.h"

#include "nodebase.h"
#include "nodeinterpreter.h"

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        Reflect::Result NodeInterpretHandleBase::read(const NodeBase &node, Reflect::Value &retVal, uint32_t dataInIndex, uint32_t group)
        {
            Pin pin = node.dataInSource(dataInIndex, group);
            return mInterpreter.read(retVal, pin);
        }

        void continueExecution(NodeInterpreterStateBase &interpreter, const NodeBase &node, BehaviorReceiver &receiver, NodeDebugLocation &location)
        {
            Pin pin = node.flowOutTarget(0);
            interpreter.branch(receiver, pin, location);
        }

    }
}
}