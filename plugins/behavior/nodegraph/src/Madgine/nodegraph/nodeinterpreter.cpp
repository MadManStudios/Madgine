#include "../nodegraphlib.h"

#include "nodeinterpreter.h"

#include "Madgine/debug/debugger.h"

#include "nodeexecution.h"
#include "nodegraph.h"

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        NodeInterpreterStateBase::NodeInterpreterStateBase(const NodeGraph *graph, NodeGraphLoader::Handle handle)
            : mDebugLocation(this)
            , mGraph(graph)
            , mHandle(std::move(handle))
        {
        }

        void NodeInterpreterStateBase::branch(BehaviorReceiver &receiver, uint32_t flowIn, NodeDebugLocation &location)
        {
            branch(receiver, mGraph->mFlowOutPins[flowIn].mTarget, location);
        }

        void NodeInterpreterStateBase::branch(BehaviorReceiver &receiver, Pin pin, NodeDebugLocation &location)
        {

            const NodeBase *node = nullptr;
            if (pin && pin.mNode) {
                node = mGraph->node(pin.mNode);
            }

            location.mNode = node;

            Debug::get_debug_context(receiver).pass(&mDebugLocation, receiver, [=, this, &location](BehaviorReceiver &receiver) {
                if (pin && pin.mNode) {
                    node->interpret({ { { { *this }, *node } }, receiver, location }, mData[pin.mNode - 1], pin.mIndex, pin.mGroup);
                } else {
                    receiver.set_value();
                } }, mContinuation, Debug::ContinuationType::Flow);
        }

        Reflect::Result NodeInterpreterStateBase::read(Reflect::Value &retVal, Pin pin)
        {
            if (!pin) {
                throw 0;
            } else if (!pin.mNode) {
                assert(pin.mGroup < 2);
                if (pin.mGroup == 0) {
                    return mArguments.get(retVal, pin.mIndex);
                } else {
                    std::string_view name = mGraph->mNamedInputs[pin.mIndex].mDescriptor.mName;
                    return get_named_d(*this, name, retVal);
                }
            } else {
                return mGraph->node(pin.mNode)->interpretRead(*this, retVal, mData[pin.mNode - 1], pin.mIndex, pin.mGroup);
            }
        }

        Reflect::Result NodeInterpreterStateBase::read(Reflect::Value &retVal, uint32_t dataProvider)
        {
            return read(retVal, mGraph->mDataInPins[dataProvider].mSource);
        }

        const NodeGraph *NodeInterpreterStateBase::graph() const
        {
            return mGraph;
        }

        std::unique_ptr<NodeInterpreterData> &NodeInterpreterStateBase::data(uint32_t index)
        {
            return mData[index - 1];
        }

        /* bool NodeInterpreterStateBase::readVar(ValueType &result, std::string_view name, bool recursive)
        {
            bool gotValue = false;
            for (const std::unique_ptr<NodeInterpreterData> &data : mData) {
                if (data) {
                    if (data->readVar(result, name)) {
                        assert(!gotValue);
                        gotValue = true;
                    }
                }
            }
            return gotValue;
        }

        bool NodeInterpreterStateBase::writeVar(std::string_view name, const ValueType &v)
        {
            bool gotValue = false;
            for (const std::unique_ptr<NodeInterpreterData> &data : mData) {
                if (data) {
                    if (data->writeVar(name, v)) {
                        assert(!gotValue);
                        gotValue = true;
                    }
                }
            }
            return gotValue;
        }

        std::vector<std::string_view> NodeInterpreterStateBase::variables()
        {
            std::vector<std::string_view> variables;
            for (const std::unique_ptr<NodeInterpreterData> &data : mData) {
                if (data) {
                    std::ranges::move(data->variables(), std::inserter(variables, variables.end()));
                }
            }
            return variables;
        }*/

        void NodeInterpreterStateBase::start()
        {
            if (!mGraph)
                mGraph = mHandle;
            mData.resize(mGraph->nodes().size());

            for (size_t i = 0; i < mData.size(); ++i) {
                mGraph->node(i + 1)->setupInterpret(*this, mData[i]);
            }

            branch(*this, 0, mDebugLocation);
        }

        void NodeInterpreterStateBase::stop()
        {
            // Nothing to do. It is the responsibility of child behaviors to listen to the stop source.
        }

    }
}
}