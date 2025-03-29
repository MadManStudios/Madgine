#include "../../nodegraphlib.h"

#include "pumpnode.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "../nodeinterpreter.h"

#include "../nodegraph.h"

#include "util/sendernode.h"

NODE(PumpNode, Engine::NodeGraph::PumpNode)

METATABLE_BEGIN_BASE(Engine::NodeGraph::PumpNode, Engine::NodeGraph::NodeBase)
METATABLE_END(Engine::NodeGraph::PumpNode)

SERIALIZETABLE_INHERIT_BEGIN(Engine::NodeGraph::PumpNode, Engine::NodeGraph::NodeBase)
SERIALIZETABLE_END(Engine::NodeGraph::PumpNode)

namespace Engine {
namespace NodeGraph {

    PumpNode::PumpNode(NodeGraph &graph)
        : Node(graph)
    {
        setup();
    }

    PumpNode::PumpNode(const PumpNode &other, NodeGraph &graph)
        : Node(other, graph)
    {
    }

    uint32_t PumpNode::flowInCount(uint32_t group) const
    {
        return 1;
    }

    uint32_t PumpNode::flowOutBaseCount(uint32_t group) const
    {
        return 1;
    }

    uint32_t PumpNode::dataInBaseCount(uint32_t group) const
    {
        return 1;
    }

    ExtendedValueTypeDesc PumpNode::dataInType(uint32_t index, uint32_t group, bool bidir) const
    {
        assert(index == 0);
        ExtendedValueTypeDesc desc { ExtendedValueTypeEnum::GenericType };
        if (mDataOutPins[0][0].mTarget)
            desc = mGraph.dataReceiverType(mDataOutPins[0][0].mTarget, false);
        if (bidir && desc.mType == ExtendedValueTypeEnum::GenericType)
            return dataOutType(index, group, false);
        return desc;
    }

    uint32_t PumpNode::dataOutBaseCount(uint32_t group) const
    {
        return 1;
    }

    ExtendedValueTypeDesc PumpNode::dataOutType(uint32_t index, uint32_t group, bool bidir) const
    {
        assert(index == 0);
        ExtendedValueTypeDesc desc { ExtendedValueTypeEnum::GenericType };
        if (mDataInPins[0][0].mSource)
            desc = mGraph.dataProviderType(mDataInPins[0][0].mSource, false);
        if (bidir && desc.mType == ExtendedValueTypeEnum::GenericType)
            return dataInType(index, group, false);
        return desc;
    }

    void PumpNode::interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const
    {
        ValueType v;
        receiver.read(v, 0);
        receiver.write(v, 0);
        receiver.set_value();
    }

}
}
