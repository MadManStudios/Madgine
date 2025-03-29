#include "../../nodegraphlib.h"

#include "testnode.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "../nodeinterpreter.h"

#include "Meta/keyvalue/valuetype.h"

#include "util/sendernode.h"

NODE(TestNode, Engine::NodeGraph::TestNode)

METATABLE_BEGIN_BASE(Engine::NodeGraph::TestNode, Engine::NodeGraph::NodeBase)
MEMBER(mDummy)
METATABLE_END(Engine::NodeGraph::TestNode)

SERIALIZETABLE_INHERIT_BEGIN(Engine::NodeGraph::TestNode, Engine::NodeGraph::NodeBase)
SERIALIZETABLE_END(Engine::NodeGraph::TestNode)

namespace Engine {
namespace NodeGraph {

    TestNode::TestNode(NodeGraph &graph)
        : Node(graph)
    {
        setup();
    }

    TestNode::TestNode(const TestNode &other, NodeGraph &graph)
        : Node(other, graph)
    {
    }

    uint32_t TestNode::flowInCount(uint32_t group) const
    {
        return 1;
    }

    uint32_t TestNode::flowOutBaseCount(uint32_t group) const
    {
        return 1;
    }

    uint32_t TestNode::dataInBaseCount(uint32_t group) const
    {
        return 2;
    }

    std::string_view TestNode::dataInName(uint32_t index, uint32_t group) const
    {
        return "data_in";
    }

    ExtendedValueTypeDesc TestNode::dataInType(uint32_t index, uint32_t group, bool bidir) const
    {
        return { ValueTypeIndex { index == 0 ? ValueTypeEnum::IntValue : ValueTypeEnum::FloatValue } };
    }

    uint32_t TestNode::dataOutBaseCount(uint32_t group) const
    {
        return 1;
    }

    std::string_view TestNode::dataOutName(uint32_t index, uint32_t group) const
    {
        return "data_out";
    }

    ExtendedValueTypeDesc TestNode::dataOutType(uint32_t index, uint32_t group, bool bidir) const
    {
        return { ValueTypeIndex { ValueTypeEnum::FloatValue } };
    }

    void TestNode::interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const
    {
        ValueType v;
        receiver.read(v, 1);
        ValueType v2;
        receiver.read(v2, 0);
        float f = v.as<float>();
        int i = v2.as<int>();
        LOG(i << " * " << f << " -> " << f * i);
        receiver.write(ValueType { i * f }, 0);
        receiver.set_value();
    }

}
}
