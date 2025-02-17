#include "../../../nodegraphlib.h"

#include "librarynode.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/keyvalueutil/valuetypeserialize.h"

#include "../../nodeinterpreter.h"

#include "../../nodeexecution.h"

METATABLE_BEGIN_BASE(Engine::NodeGraph::LibraryNode, Engine::NodeGraph::NodeBase)
MEMBER(mParameters)
METATABLE_END(Engine::NodeGraph::LibraryNode)

SERIALIZETABLE_INHERIT_BEGIN(Engine::NodeGraph::LibraryNode, Engine::NodeGraph::NodeBase)
FIELD(mParameters)
SERIALIZETABLE_END(Engine::NodeGraph::LibraryNode)

namespace Engine {
namespace NodeGraph {

    struct LibraryInterpretReceiver {
        void set_value(ArgumentList values)
        {
            mResult = std::move(values);
            NodeReceiver<NodeBase> receiver = std::move(*mReceiver);
            mReceiver.reset();
            receiver.set_value();
        }

        void set_error(BehaviorError result)
        {
            NodeReceiver<NodeBase> receiver = std::move(*mReceiver);
            mReceiver.reset();
            receiver.set_error(std::move(result));
        }

        void set_done()
        {
            NodeReceiver<NodeBase> receiver = std::move(*mReceiver);
            mReceiver.reset();
            receiver.set_done();
        }

        template <typename CPO, typename... Args>
        friend auto tag_invoke(CPO f, LibraryInterpretReceiver &rec, Args &&...args)
            -> tag_invoke_result_t<CPO, BehaviorReceiver &, Args...>
        {
            return f(*rec.mReceiver, std::forward<Args>(args)...);
        }

        std::optional<NodeReceiver<NodeBase>> mReceiver;
        ArgumentList mResult;
    };

    struct LibraryInterpretData : NodeInterpreterData, Execution::VirtualState<LibraryInterpretReceiver, BehaviorReceiver> {

        static std::vector<Behavior> buildSubBehaviors(size_t count)
        {
            std::vector<Behavior> result;
            /* std::array<Behavior (*)(), 5> ctors = {
                []() -> Behavior { return NodeSender<0> {}; },
                []() -> Behavior { return NodeSender<1> {}; },
                []() -> Behavior { return NodeSender<2> {}; },
                []() -> Behavior { return NodeSender<3> {}; },
                []() -> Behavior { return NodeSender<4> {}; }
            };
            assert(count < ctors.size());
            for (size_t i = 0; i < count; ++i) {
                result.push_back(ctors[i]);
            }*/
            return result;
        }

        LibraryInterpretData(BehaviorHandle type, const ParameterTuple &args, size_t subBehaviorCount)
            : Execution::VirtualState<LibraryInterpretReceiver, BehaviorReceiver>(LibraryInterpretReceiver {})
            , mBehavior(type.create(args, buildSubBehaviors(subBehaviorCount)).connect(*this))
        {
        }

        void start(NodeReceiver<NodeBase> receiver)
        {
            mRec.mReceiver.emplace(std::move(receiver));
            mBehavior->start();
        }

        Behavior::StatePtr mBehavior;
    };

    LibraryNode::LibraryNode(NodeGraph &graph, BehaviorHandle behavior)
        : VirtualData(graph)
        , mBehavior(std::move(behavior))
        , mParameters(mBehavior.createDummyParameters())
        , mFullClassName(mBehavior.toString())
    {
        Execution::detach(mBehavior.state().sender() | Execution::then([this](bool success) {
            if (success) {
                mBindings = mBehavior.bindings();
                mSubBehaviorCount = mBehavior.subBehaviorCount();
                this->setup();
            } else
                LOG_ERROR("TODO");
        }));
    }

    LibraryNode::LibraryNode(const LibraryNode &other, NodeGraph &graph)
        : VirtualData(other, graph)
        , mBehavior(other.mBehavior)
        , mParameters(other.mParameters)
        , mFullClassName(other.mFullClassName)
        , mBindings(other.mBindings)
    {
    }

    std::string_view LibraryNode::name() const
    {
        return mBehavior.name();
    }

    std::string_view LibraryNode::className() const
    {
        return mFullClassName;
    }

    std::unique_ptr<NodeBase> LibraryNode::clone(NodeGraph &graph) const
    {
        return std::make_unique<LibraryNode>(*this, graph);
    }

    uint32_t LibraryNode::flowInCount(uint32_t group) const
    {
        return 1;
    }

    uint32_t LibraryNode::flowOutGroupCount() const
    {
        return 2;
    }

    uint32_t LibraryNode::flowOutBaseCount(uint32_t group) const
    {
        if (group == 0)
            return 1;
        else
            return mSubBehaviorCount;
    }

    std::string_view LibraryNode::flowOutName(uint32_t index, uint32_t group) const
    {
        if (group == 0)
            return NodeBase::flowOutName(index, group);
        else
            return "Sub Behavior";
    }

    uint32_t LibraryNode::dataInGroupCount() const
    {
        return 2;
    }

    uint32_t LibraryNode::dataInBaseCount(uint32_t group) const
    {
        if (group == 0) {
            return 0;
        } else {
            return mBindings.size();
        }
    }

    std::string_view LibraryNode::dataInName(uint32_t index, uint32_t group) const
    {
        if (group == 0) {
            throw 0;
        } else {
            if (index >= mBindings.size())
                return "<unknown>";
            return mBindings[index].mName;
        }
    }

    ExtendedValueTypeDesc LibraryNode::dataInType(uint32_t index, uint32_t group, bool bidir) const
    {
        if (group == 0) {
            throw 0;
        } else {
            if (index >= mBindings.size())
                return { ExtendedValueTypeEnum::GenericType };
            return mBindings[index].mType;
        }
    }

    uint32_t LibraryNode::dataProviderBaseCount(uint32_t group) const
    {
        return mBehavior.resultTypes().size();
    }

    ExtendedValueTypeDesc LibraryNode::dataProviderType(uint32_t index, uint32_t group, bool bidir) const
    {
        return mBehavior.resultTypes()[index];
    }

    void LibraryNode::setupInterpret(NodeInterpreterStateBase &interpreter, std::unique_ptr<NodeInterpreterData> &data) const
    {
        data = std::make_unique<LibraryInterpretData>(mBehavior, mParameters, mSubBehaviorCount);
    }

    void LibraryNode::interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const
    {
        static_cast<LibraryInterpretData *>(data.get())->start(std::move(receiver));
    }

    BehaviorError LibraryNode::interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const
    {
        retVal = static_cast<LibraryInterpretData *>(data.get())->mRec.mResult[providerIndex];
        return {};
    }

}
}