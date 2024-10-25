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

    struct LibraryInterpretData : NodeInterpreterData, BehaviorReceiver {

        LibraryInterpretData(BehaviorHandle type, const ParameterTuple &args)
            : mBehavior(type.create(args).connect(*this))
        {
        }

        void start(NodeReceiver<NodeBase> receiver)
        {
            mReceiver.emplace(std::move(receiver));
            mBehavior->start();
        }

        void set_value(ArgumentList values) override
        {
            mResult = std::move(values);
            NodeReceiver<NodeBase> receiver = std::move(*mReceiver);
            mReceiver.reset();
            receiver.set_value();
        }

        void set_error(BehaviorError result) override
        {
            NodeReceiver<NodeBase> receiver = std::move(*mReceiver);
            mReceiver.reset();
            receiver.set_error(std::move(result));
        }

        void set_done() override
        {
            NodeReceiver<NodeBase> receiver = std::move(*mReceiver);
            mReceiver.reset();
            receiver.set_done();
        }

        BehaviorError getBinding(std::string_view name, ValueType &ref) override
        {
            for (uint32_t i = 0; i < mReceiver->mNode.dataInCount(1); ++i) {
                if (mReceiver->mNode.dataInName(i, 1) == name) {
                    return mReceiver->read(ref, i, 1);
                }
            }
            throw 0;
            //return false;
        }

        Debug::ParentLocation *debugLocation() override
        {
            return Execution::get_debug_location(*mReceiver);
        }

        std::stop_token stopToken() override
        {
            return Execution::get_stop_token(*mReceiver);
        }

        Log::Log *log() override
        {
            return Log::get_log(*mReceiver);
        }

        Behavior::StatePtr mBehavior;
        std::optional<NodeReceiver<NodeBase>> mReceiver;
        ArgumentList mResult;
    };

    LibraryNode::LibraryNode(NodeGraph &graph, BehaviorHandle behavior)
        : VirtualData(graph)
        , mBehavior(std::move(behavior))
        , mParameters(mBehavior.createDummyParameters())
        , mFullClassName(mBehavior.toString())
    {
        this->setup();

        Execution::detach(mBehavior.state().sender() | Execution::then([this](bool success) {
            mBindings = mBehavior.bindings();
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

    uint32_t LibraryNode::flowOutBaseCount(uint32_t group) const
    {
        return 1;
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
        data = std::make_unique<LibraryInterpretData>(mBehavior, mParameters);
    }

    void LibraryNode::interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const
    {
        static_cast<LibraryInterpretData *>(data.get())->start(std::move(receiver));
    }

    BehaviorError LibraryNode::interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const
    {
        retVal = static_cast<LibraryInterpretData *>(data.get())->mResult[providerIndex];
        return {};
    }

}
}