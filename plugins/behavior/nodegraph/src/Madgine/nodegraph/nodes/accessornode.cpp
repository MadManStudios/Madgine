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
// PROPERTY(Function, getFunction, setFunction)
METATABLE_END(Engine::NodeGraph::AccessorNode)

SERIALIZETABLE_INHERIT_BEGIN(Engine::NodeGraph::AccessorNode, Engine::NodeGraph::NodeBase)
// ENCAPSULATED_FIELD(Function, getFunctionName, setFunctionName)
SERIALIZETABLE_END(Engine::NodeGraph::AccessorNode)

namespace Engine {
namespace NodeGraph {

    AccessorNode::AccessorNode(NodeGraph &graph, std::string_view fullClassName)
        : VirtualData(graph)
        , mFullClassName { fullClassName }
    {
        setup();
    }

    AccessorNode::AccessorNode(const AccessorNode &other, NodeGraph &graph)
        : VirtualData(other, graph)
        , mFullClassName(other.mFullClassName)
    {
    }

    std::string_view AccessorNode::name() const
    {
        return std::string_view { mFullClassName }.substr(mFullClassName.rfind('/') + 1);
    }

    std::string_view AccessorNode::className() const
    {
        return mFullClassName;
    }

    std::unique_ptr<NodeBase> AccessorNode::clone(NodeGraph &graph) const
    {
        return std::make_unique<AccessorNode>(*this, graph);
    }
        
    uint32_t AccessorNode::flowInCount(uint32_t group) const
    {
        return accessor()->mType.mType == ValueTypeEnum::ApiFunctionValue || accessor()->mType.mType == ValueTypeEnum::BoundApiFunctionValue ? 1 : 0;
    }

    uint32_t AccessorNode::flowOutBaseCount(uint32_t group) const
    {
        return accessor()->mType.mType == ValueTypeEnum::ApiFunctionValue || accessor()->mType.mType == ValueTypeEnum::BoundApiFunctionValue ? 1 : 0;
    }

    uint32_t AccessorNode::dataInBaseCount(uint32_t group) const
    {
        if (accessor()->mType.mType == ValueTypeEnum::ApiFunctionValue || accessor()->mType.mType == ValueTypeEnum::BoundApiFunctionValue) {
            return (*accessor()->mType.mSecondary.mFunctionTable)->mArgumentsCount;
        } else {
            return 1;
        }        
    }

    std::string_view AccessorNode::dataInName(uint32_t index, uint32_t group) const
    {
        if (accessor()->mType.mType == ValueTypeEnum::ApiFunctionValue || accessor()->mType.mType == ValueTypeEnum::BoundApiFunctionValue) {
            return (*accessor()->mType.mSecondary.mFunctionTable)->mArguments[index].mName;
        } else {
            return "this";
        }
    }

    ExtendedValueTypeDesc AccessorNode::dataInType(uint32_t index, uint32_t group, bool bidir) const
    {
        if (accessor()->mType.mType == ValueTypeEnum::ApiFunctionValue || accessor()->mType.mType == ValueTypeEnum::BoundApiFunctionValue) {
            return (*accessor()->mType.mSecondary.mFunctionTable)->mArguments[index].mType;
        } else {
            return { { ValueTypeEnum::ScopeValue }, type()->mSelf };
        }
    }

    uint32_t AccessorNode::dataProviderBaseCount(uint32_t group) const
    {
        return 1;
    }

    ExtendedValueTypeDesc AccessorNode::dataProviderType(uint32_t index, uint32_t group, bool bidir) const
    {
        if (accessor()->mType.mType == ValueTypeEnum::ApiFunctionValue || accessor()->mType.mType == ValueTypeEnum::BoundApiFunctionValue) {
            return (*accessor()->mType.mSecondary.mFunctionTable)->mReturnType;
        } else {
            return accessor()->mType;
        }
    }

    BehaviorError AccessorNode::interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const
    {
        if (accessor()->mType.mType == ValueTypeEnum::ApiFunctionValue || accessor()->mType.mType == ValueTypeEnum::BoundApiFunctionValue) {
            ArgumentList arguments { dataInCount() };
            for (size_t i = 0; i < dataInCount(); ++i) {
                BehaviorError error = NodeInterpretHandle<NodeBase> { interpreter, *this }.read(arguments[i], i);
                if (error.mResult != GenericResult::SUCCESS) {
                    return error;
                }
            }
            (*accessor()->mType.mSecondary.mFunctionTable)->mFunctionPtr((*accessor()->mType.mSecondary.mFunctionTable), retVal, arguments);
        } else {
            ValueType scope;
            if (BehaviorError error = NodeInterpretHandle<NodeBase> { interpreter, *this }.read(scope, 0); error.mResult != GenericResult::SUCCESS)
                return error;

            accessor()->mGetter(accessor(), retVal, scope.as<ScopePtr>());            
        }
        return {};
    }

    CodeGen::Statement AccessorNode::generateRead(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t providerIndex, uint32_t group) const
    {
        throw 0;
    }

    const MetaTable *AccessorNode::type() const
    {
        std::string_view fullClassName = mFullClassName;

        std::string_view path = fullClassName.substr(strlen("Accessor/"));

        auto pos = path.find("/");
        if (pos == std::string_view::npos)
            return nullptr;

        std::string_view typeName = path.substr(0, pos);

        const MetaTable *type = sTypeList();
        while (type) {
            if (type->mTypeName == typeName) {
                return type;
            }
            type = type->mNext;
        }
        return nullptr;
    }

    const Accessor *AccessorNode::accessor() const
    {
        std::string_view fullClassName = mFullClassName;

        std::string_view path = fullClassName.substr(strlen("Accessor/"));

        auto pos = path.find("/");
        if (pos == std::string_view::npos)
            return nullptr;

        std::string_view typeName = path.substr(0, pos);
        std::string_view accessorName = path.substr(pos + 1);

        const MetaTable *classType = type();

        for (const Accessor *accessor = classType->mMembers; accessor->mName; ++accessor) {
            if (accessor->mName == accessorName) {
                return accessor;
            }
        }

        return nullptr;
    }

}
}
