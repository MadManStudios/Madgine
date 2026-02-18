#include "../../nodegraphlib.h"

#include "behaviornode.h"

#include "Meta/keyvalue/valuetype.h"
#include "Meta/keyvalueutil/valuetypeserialize.h"

#include "Madgine/resources/resourcemanager.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../nodeexecution.h"
#include "../nodeinterpreter.h"

METATABLE_BEGIN_BASE(Engine::Behavior::NodeGraph::BehaviorNode, Engine::Behavior::NodeGraph::NodeBase)
    MEMBER(mParameters)
METATABLE_END(Engine::Behavior::NodeGraph::BehaviorNode)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Behavior::NodeGraph::BehaviorNode, Engine::Behavior::NodeGraph::NodeBase)
    FIELD(mParameters)
SERIALIZETABLE_END(Engine::Behavior::NodeGraph::BehaviorNode)

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        struct BehaviorInterpretReceiver {
            void set_value(ArgumentList values)
            {
                mBehavior.reset();
                mResult = std::move(values);
                NodeReceiver<NodeBase> receiver = std::move(*mReceiver);
                Execution::get_debug_location(receiver)->stepOut(mLocation);
                mReceiver.reset();
                receiver.set_value();
            }

            void set_error(BehaviorError result)
            {
                mBehavior.reset();
                NodeReceiver<NodeBase> receiver = std::move(*mReceiver);
                Execution::get_debug_location(receiver)->stepOut(mLocation);
                mReceiver.reset();
                receiver.set_error(std::move(result));
            }

            void set_done()
            {
                mBehavior.reset();
                NodeReceiver<NodeBase> receiver = std::move(*mReceiver);
                Execution::get_debug_location(receiver)->stepOut(mLocation);
                mReceiver.reset();
                receiver.set_done();
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, BehaviorInterpretReceiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, NodeReceiver<NodeBase> &, Args...>
            {
                return f(*rec.mReceiver, std::forward<Args>(args)...);
            }

            friend Debug::SenderLocation *tag_invoke(Execution::get_debug_location_t, BehaviorInterpretReceiver &rec)
            {
                return &rec.mLocation;
            }

            std::optional<NodeReceiver<NodeBase>> mReceiver;
            Behavior::StatePtr mBehavior;
            ArgumentList mResult;
            Debug::SenderLocation mLocation { [](CallableView<void(const Execution::StateDescriptor &)>) -> void { throw 0; } };
        };

        struct BehaviorInterpretData : NodeInterpreterData, Execution::VirtualState<BehaviorReceiver, BehaviorInterpretReceiver> {

            static std::vector<Behavior> buildSubBehaviors(uint32_t count, const NodeInterpretHandle<BehaviorNode> &handle)
            {
                std::vector<Behavior> result;
                for (uint32_t i = 0; i < count; ++i) {
                    result.push_back(NodeSender<1> { i } | NodeReceiverWrapper { handle });
                }
                return result;
            }

            BehaviorInterpretData(BehaviorHandle type)
                : Execution::VirtualState<BehaviorReceiver, BehaviorInterpretReceiver>(BehaviorInterpretReceiver {})
                , mType(type)
            {
            }

            void start(NodeReceiver<NodeBase> receiver, const ParameterTuple &args)
            {
                NodeInterpretHandle<BehaviorNode> handle { { receiver.mInterpreter }, static_cast<const BehaviorNode &>(receiver.mNode) };

                mRec.mReceiver.emplace(std::move(receiver));

                Execution::get_debug_location(*mRec.mReceiver)->stepInto(mRec.mLocation);

                mRec.mBehavior = mType.create(args, buildSubBehaviors(mType.subBehaviorCount(), handle)).connect(*this);
                mRec.mBehavior->start();
            }

            BehaviorHandle mType;
        };

        BehaviorNode::BehaviorNode(NodeGraph &graph, BehaviorHandle behavior, Threading::TaskFuture<bool> &future)
            : VirtualData(graph)
            , mBehavior(std::move(behavior))
            , mFullClassName(mBehavior.toString())
            , mParameters(mBehavior.createParameters())
        {
            future = Engine::Resources::ResourceManager::getSingleton().taskQueue()->queueTask(mBehavior.state().then([this](bool success) {
                if (success) {
                    mNamedInputs = mBehavior.namedInputs();
                    mSubBehaviorCount = mBehavior.subBehaviorCount();
                    this->setup();
                }
                return success;
            }));
        }

        BehaviorNode::BehaviorNode(NodeGraph &graph, BehaviorHandle behavior)
            : VirtualData(graph)
            , mBehavior(std::move(behavior))
            , mFullClassName(mBehavior.toString())
            , mParameters(mBehavior.createParameters())            
        {
            assert(mBehavior.state());
            mNamedInputs = mBehavior.namedInputs();
            mSubBehaviorCount = mBehavior.subBehaviorCount();
            this->setup();
        }

        BehaviorNode::BehaviorNode(const BehaviorNode &other, NodeGraph &graph)
            : VirtualData(other, graph)
            , mBehavior(other.mBehavior)
            , mFullClassName(other.mFullClassName)
            , mParameters(other.mParameters)
            , mNamedInputs(other.mNamedInputs)
        {
        }

        std::string_view BehaviorNode::name() const
        {
            return mBehavior.name();
        }

        std::string_view BehaviorNode::className() const
        {
            return mFullClassName;
        }

        std::unique_ptr<NodeBase> BehaviorNode::clone(NodeGraph &graph) const
        {
            return std::make_unique<BehaviorNode>(*this, graph);
        }

        uint32_t BehaviorNode::flowInCount(uint32_t group) const
        {
            return 1;
        }

        uint32_t BehaviorNode::flowOutGroupCount() const
        {
            return 2;
        }

        uint32_t BehaviorNode::flowOutBaseCount(uint32_t group) const
        {
            if (group == 0)
                return 1;
            else
                return mSubBehaviorCount;
        }

        std::string_view BehaviorNode::flowOutName(uint32_t index, uint32_t group) const
        {
            if (group == 0)
                return NodeBase::flowOutName(index, group);
            else
                return "Sub Behavior";
        }

        uint32_t BehaviorNode::dataInGroupCount() const
        {
            return 2;
        }

        uint32_t BehaviorNode::dataInBaseCount(uint32_t group) const
        {
            if (group == 0) {
                return 0;
            } else {
                return mNamedInputs.size();
            }
        }

        std::string_view BehaviorNode::dataInName(uint32_t index, uint32_t group) const
        {
            if (group == 0) {
                throw 0;
            } else {
                if (index >= mNamedInputs.size())
                    return "<unknown>";
                return mNamedInputs[index].mName;
            }
        }

        ExtendedValueTypeDesc BehaviorNode::dataInType(uint32_t index, uint32_t group, bool bidir) const
        {
            if (group == 0) {
                throw 0;
            } else {
                if (index >= mNamedInputs.size())
                    return { ExtendedValueTypeEnum::GenericType };
                return mNamedInputs[index].mType;
            }
        }

        uint32_t BehaviorNode::dataOutBaseCount(uint32_t group) const
        {
            return mBehavior.resultTypes().size();
        }

        ExtendedValueTypeDesc BehaviorNode::dataOutType(uint32_t index, uint32_t group, bool bidir) const
        {
            return mBehavior.resultTypes()[index];
        }

        void BehaviorNode::setupInterpret(NodeInterpreterStateBase &interpreter, std::unique_ptr<NodeInterpreterData> &data) const
        {
            data = std::make_unique<BehaviorInterpretData>(mBehavior);
        }

        void BehaviorNode::interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const
        {
            static_cast<BehaviorInterpretData *>(data.get())->start(std::move(receiver), mParameters);
        }

        BehaviorError BehaviorNode::interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const
        {
            retVal = static_cast<BehaviorInterpretData *>(data.get())->mRec.mResult[providerIndex];
            return {};
        }

    }
}
}