#include "../../nodegraphlib.h"

#include "functionnode.h"

#include "Meta/reflect/functiontable.h"
#include "Meta/reflect/value.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../nodeexecution.h"
#include "../nodeinterpreter.h"

METATABLE_BEGIN_BASE(Engine::Behavior::NodeGraph::FunctionNode, Engine::Behavior::NodeGraph::NodeBase)
// PROPERTY(Function, getFunction, setFunction)
METATABLE_END(Engine::Behavior::NodeGraph::FunctionNode)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Behavior::NodeGraph::FunctionNode, Engine::Behavior::NodeGraph::NodeBase)
// ENCAPSULATED_FIELD(Function, getFunctionName, setFunctionName)
SERIALIZETABLE_END(Engine::Behavior::NodeGraph::FunctionNode)

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        struct FunctionInterpretData : NodeInterpreterData {

            FunctionInterpretData()
            {
            }

            Reflect::Value mResult;
        };

        FunctionNode::FunctionNode(NodeGraph &graph, std::string_view fullClassName)
            : VirtualData(graph)
            , mFullClassName { fullClassName }
        {
            refresh();
        }

        FunctionNode::FunctionNode(const FunctionNode &other, NodeGraph &graph)
            : VirtualData(other, graph)
            , mFullClassName(other.mFullClassName)
        {
        }

        std::string_view FunctionNode::name() const
        {
            return std::string_view { mFullClassName }.substr(mFullClassName.rfind('/') + 1);
        }

        std::string_view FunctionNode::className() const
        {
            return mFullClassName;
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
            return function() ? function()->mArgumentsCount : 0;
        }

        std::string_view FunctionNode::dataInName(uint32_t index, uint32_t group) const
        {
            return function()->mArguments[index].mName;
        }

        Reflect::ExtendedType FunctionNode::dataInType(uint32_t index, uint32_t group, bool bidir) const
        {
            return function()->mArguments[index].mType;
        }

        uint32_t FunctionNode::dataOutBaseCount(uint32_t group) const
        {
            return 1;
        }

        Reflect::ExtendedType FunctionNode::dataOutType(uint32_t index, uint32_t group, bool bidir) const
        {
            return function()->mReturnType;
        }

        void FunctionNode::setupInterpret(NodeInterpreterStateBase &interpreter, std::unique_ptr<NodeInterpreterData> &data) const
        {
            data = std::make_unique<FunctionInterpretData>();
        }

        void FunctionNode::interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const
        {
            Reflect::ArgumentList arguments { std::true_type {}, dataInCount() };
            for (size_t i = 0; i < dataInCount(); ++i) {
                Reflect::Result result = receiver.read(arguments[i], i);
                if (result) {
                    receiver.set_error(std::move(*result.mError));
                    return;
                }
            }
            Reflect::Result result = function()->mFunctionPtr(function(), static_cast<FunctionInterpretData &>(*data).mResult, arguments, {});
            if (result) {
                receiver.set_error(std::move(*result.mError));
            } else {
                receiver.set_value();
            }
        }

        Reflect::Result FunctionNode::interpretRead(NodeInterpreterStateBase &interpreter, Reflect::Value &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const
        {
            retVal = static_cast<FunctionInterpretData *>(data.get())->mResult;
            return {};
        }

        const Reflect::FunctionTable *FunctionNode::function() const
        {
            std::string_view fullClassName = mFullClassName;

            std::string_view path = fullClassName.substr(strlen("Function/"));

            for (const Reflect::FunctionTable *f = Reflect::sFunctionList(); f; f = f->mNext) {
                if (f->mName == path) {
                    return f;
                }
            }

            return nullptr;
        }

    }
}
}