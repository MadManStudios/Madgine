#include "../../nodegraphlib.h"

#include "functionnode.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalueutil/valuetypeserialize.h"

#include "../nodeinterpreter.h"

#include "../codegenerator.h"

#include "Meta/keyvalue/functiontable.h"

#include "../nodeexecution.h"

METATABLE_BEGIN_BASE(Engine::NodeGraph::FunctionNode, Engine::NodeGraph::NodeBase)
//PROPERTY(Function, getFunction, setFunction)
METATABLE_END(Engine::NodeGraph::FunctionNode)

SERIALIZETABLE_INHERIT_BEGIN(Engine::NodeGraph::FunctionNode, Engine::NodeGraph::NodeBase)
//ENCAPSULATED_FIELD(Function, getFunctionName, setFunctionName)
SERIALIZETABLE_END(Engine::NodeGraph::FunctionNode)

namespace Engine {
namespace NodeGraph {

    FunctionNode::FunctionNode(NodeGraph &graph, const FunctionTable *function)
        : VirtualData(graph)
        , mFunction(function)
    {
        setup();
    }

    FunctionNode::FunctionNode(const FunctionNode &other, NodeGraph &graph)
        : VirtualData(other, graph)
        , mFunction(other.mFunction)
    {
    }

    std::string_view FunctionNode::name() const
    {
        return "";
    }

    std::string_view FunctionNode::className() const
    {
        return "";
    }

    std::unique_ptr<NodeBase> FunctionNode::clone(NodeGraph &graph) const
    {
        return std::make_unique<FunctionNode>(*this, graph);
    }

    uint32_t FunctionNode::flowInCount(uint32_t group) const
    {
        return 1;
    }

    uint32_t FunctionNode::flowOutBaseCount(uint32_t group) const
    {
        return 1;
    }

    uint32_t FunctionNode::dataInBaseCount(uint32_t group) const
    {
        return mFunction ? mFunction->mArgumentsCount : 0;
    }

    std::string_view FunctionNode::dataInName(uint32_t index, uint32_t group) const
    {
        return mFunction->mArguments[index].mName;
    }

    ExtendedValueTypeDesc FunctionNode::dataInType(uint32_t index, uint32_t group, bool bidir) const
    {
        return mFunction->mArguments[index].mType;
    }

    uint32_t FunctionNode::dataProviderBaseCount(uint32_t group) const
    {
        return 1;
    }

    ExtendedValueTypeDesc FunctionNode::dataProviderType(uint32_t index, uint32_t group, bool bidir) const
    {
        return mFunction->mReturnType;
    }

    BehaviorError FunctionNode::interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const
    {
        ArgumentList arguments { dataInCount() };
        for (size_t i = 0; i < dataInCount(); ++i) {
            BehaviorError error = NodeInterpretHandle<NodeBase> { interpreter, *this }.read(arguments[i], i);
            if (error.mResult != GenericResult::SUCCESS) {
                return error;
            }
        }
        mFunction->mFunctionPtr(mFunction, retVal, arguments);
        return {};
    }

    CodeGen::Statement FunctionNode::generateRead(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t providerIndex, uint32_t group) const
    {
        throw 0;
    }

    const FunctionTable *FunctionNode::getFunction() const
    {
        return mFunction;
    }

    std::string_view FunctionNode::getFunctionName() const
    {
        return mFunction ? mFunction->mName : "";
    }

}
}
