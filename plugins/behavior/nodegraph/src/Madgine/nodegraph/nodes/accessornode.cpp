#include "../../nodegraphlib.h"

#include "accessornode.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalueutil/valuetypeserialize.h"

#include "../nodeinterpreter.h"

#include "../codegenerator.h"

#include "Meta/keyvalue/functiontable.h"

#include "../nodeexecution.h"

METATABLE_BEGIN_BASE(Engine::NodeGraph::AccessorNode, Engine::NodeGraph::NodeBase)
//PROPERTY(Function, getFunction, setFunction)
METATABLE_END(Engine::NodeGraph::AccessorNode)

SERIALIZETABLE_INHERIT_BEGIN(Engine::NodeGraph::AccessorNode, Engine::NodeGraph::NodeBase)
//ENCAPSULATED_FIELD(Function, getFunctionName, setFunctionName)
SERIALIZETABLE_END(Engine::NodeGraph::AccessorNode)

namespace Engine {
namespace NodeGraph {

    AccessorNode::AccessorNode(NodeGraph &graph, const MetaTable **type, const std::pair<const char *, Accessor> *accessor)
        : VirtualData(graph)
        , mType(type)
        , mAccessor(accessor)
        , mFullClassName { "Accessor/"s + (*type)->mTypeName + "/" + accessor->first }
    {
        setup();
    }

    AccessorNode::AccessorNode(const AccessorNode &other, NodeGraph &graph)
        : VirtualData(other, graph)
        , mType(other.mType)
        , mAccessor(other.mAccessor)
        , mFullClassName(other.mFullClassName)
    {
    }

    std::string_view AccessorNode::name() const
    {
        return mAccessor->first;
    }

    std::string_view AccessorNode::className() const
    {
        return mFullClassName;
    }

    std::unique_ptr<NodeBase> AccessorNode::clone(NodeGraph &graph) const
    {
        return std::make_unique<AccessorNode>(*this, graph);
    }

    uint32_t AccessorNode::dataInBaseCount(uint32_t group) const
    {
        return 1;
    }

    std::string_view AccessorNode::dataInName(uint32_t index, uint32_t group) const
    {
        return "this";
    }

    ExtendedValueTypeDesc AccessorNode::dataInType(uint32_t index, uint32_t group, bool bidir) const
    {
        return { { ValueTypeEnum::ScopeValue }, mType };
    }

    uint32_t AccessorNode::dataProviderBaseCount(uint32_t group) const
    {
        return 1;
    }

    ExtendedValueTypeDesc AccessorNode::dataProviderType(uint32_t index, uint32_t group, bool bidir) const
    {
        return mAccessor->second.mType;
    }

    BehaviorError AccessorNode::interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const
    {
        ValueType scope;
        if (BehaviorError error = NodeInterpretHandle<NodeBase> { interpreter, *this }.read(scope, 0); error.mResult != GenericResult::SUCCESS)
            return error;

        mAccessor->second.mGetter(retVal, scope.as<ScopePtr>());
        return {};
    }

    CodeGen::Statement AccessorNode::generateRead(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t providerIndex, uint32_t group) const
    {
        throw 0;
    }

}
}
