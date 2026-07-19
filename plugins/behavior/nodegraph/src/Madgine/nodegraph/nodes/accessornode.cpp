#include "../../nodegraphlib.h"

#include "accessornode.h"

#include "Meta/reflect/functiontable.h"
#include "Meta/reflect/value.h"
#include "Meta/reflectserialize/valuetypeserialize.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../nodeexecution.h"
#include "../nodeinterpreter.h"

METATABLE_BEGIN_BASE(Engine::Behavior::NodeGraph::AccessorNode, Engine::Behavior::NodeGraph::NodeBase)
// PROPERTY(Function, getFunction, setFunction)
METATABLE_END(Engine::Behavior::NodeGraph::AccessorNode)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Behavior::NodeGraph::AccessorNode, Engine::Behavior::NodeGraph::NodeBase)
// ENCAPSULATED_FIELD(Function, getFunctionName, setFunctionName)
SERIALIZETABLE_END(Engine::Behavior::NodeGraph::AccessorNode)

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        AccessorNode::AccessorNode(NodeGraph &graph, std::string_view fullClassName)
            : VirtualData(graph)
            , mFullClassName { fullClassName }
        {
            refresh();
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
            return accessor()->mType.mType == Reflect::TypeEnum::ApiFunctionValue || accessor()->mType.mType == Reflect::TypeEnum::BoundApiFunctionValue ? 1 : 0;
        }

        uint32_t AccessorNode::flowOutBaseCount(uint32_t group) const
        {
            return accessor()->mType.mType == Reflect::TypeEnum::ApiFunctionValue || accessor()->mType.mType == Reflect::TypeEnum::BoundApiFunctionValue ? 1 : 0;
        }

        uint32_t AccessorNode::dataInBaseCount(uint32_t group) const
        {
            if (accessor()->mType.mType == Reflect::TypeEnum::ApiFunctionValue || accessor()->mType.mType == Reflect::TypeEnum::BoundApiFunctionValue) {
                return (*accessor()->mType.mSecondary.mFunctionTable)->mArgumentsCount;
            } else {
                return 1;
            }
        }

        std::string_view AccessorNode::dataInName(uint32_t index, uint32_t group) const
        {
            if (accessor()->mType.mType == Reflect::TypeEnum::ApiFunctionValue || accessor()->mType.mType == Reflect::TypeEnum::BoundApiFunctionValue) {
                return (*accessor()->mType.mSecondary.mFunctionTable)->mArguments[index].mName;
            } else {
                return "this";
            }
        }

        Reflect::ExtendedType AccessorNode::dataInType(uint32_t index, uint32_t group, bool bidir) const
        {
            if (accessor()->mType.mType == Reflect::TypeEnum::ApiFunctionValue || accessor()->mType.mType == Reflect::TypeEnum::BoundApiFunctionValue) {
                return (*accessor()->mType.mSecondary.mFunctionTable)->mArguments[index].mType;
            } else {
                return { { Reflect::TypeEnum::ScopeValue }, type()->mSelf };
            }
        }

        uint32_t AccessorNode::dataOutBaseCount(uint32_t group) const
        {
            return 1;
        }

        Reflect::ExtendedType AccessorNode::dataOutType(uint32_t index, uint32_t group, bool bidir) const
        {
            if (accessor()->mType.mType == Reflect::TypeEnum::ApiFunctionValue || accessor()->mType.mType == Reflect::TypeEnum::BoundApiFunctionValue) {
                return (*accessor()->mType.mSecondary.mFunctionTable)->mReturnType;
            } else {
                return accessor()->mType;
            }
        }

        Reflect::Result AccessorNode::interpretRead(NodeInterpreterStateBase &interpreter, Reflect::Value &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const
        {
            if (accessor()->mType.mType == Reflect::TypeEnum::ApiFunctionValue || accessor()->mType.mType == Reflect::TypeEnum::BoundApiFunctionValue) {
                Reflect::ArgumentList arguments { std::true_type {}, dataInCount() };
                for (size_t i = 0; i < dataInCount(); ++i) {
                    REFLECT_PROPAGATE_ERROR(NodeInterpretHandle<NodeBase> { { interpreter }, *this }.read(arguments[i], i));
                }
                return (*accessor()->mType.mSecondary.mFunctionTable)->mFunctionPtr((*accessor()->mType.mSecondary.mFunctionTable), retVal, arguments);
            } else {
                Reflect::Value scope;
                REFLECT_PROPAGATE_ERROR(NodeInterpretHandle<NodeBase> { { interpreter }, *this }.read(scope, 0));

                return accessor()->mGetter(accessor(), retVal, scope);
            }
        }

        const Reflect::MetaTable *AccessorNode::type() const
        {
            std::string_view fullClassName = mFullClassName;

            std::string_view path = fullClassName.substr(strlen("Accessor/"));

            auto pos = path.find("/");
            if (pos == std::string_view::npos)
                return nullptr;

            std::string_view typeName = path.substr(0, pos);

            auto type = Type::resolveTypeName(typeName);
            if (type) {
                return type->mMetaTable;
            }
            return nullptr;
        }

        const Reflect::Accessor *AccessorNode::accessor() const
        {
            std::string_view fullClassName = mFullClassName;

            std::string_view path = fullClassName.substr(strlen("Accessor/"));

            auto pos = path.find("/");
            if (pos == std::string_view::npos)
                return nullptr;

            std::string_view accessorName = path.substr(pos + 1);

            const Reflect::MetaTable *classType = type();

            for (const Reflect::Accessor *accessor = classType->mMembers; accessor->mName; ++accessor) {
                if (accessor->mName == accessorName) {
                    return accessor;
                }
            }

            return nullptr;
        }

    }
}
}
