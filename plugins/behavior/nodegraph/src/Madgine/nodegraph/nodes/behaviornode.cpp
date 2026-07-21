#include "../../nodegraphlib.h"

#include "behaviornode.h"

#include "Madgine/behavior/behaviordescriptor.h"
#include "Madgine/resources/resourcemanager.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "../nodeexecution.h"
#include "../nodeinterpreter.h"

METATABLE_BEGIN_BASE(Engine::Behavior::NodeGraph::BehaviorNode, Engine::Behavior::NodeGraph::NodeBase)
    MEMBER(mDefaultParameters)
METATABLE_END(Engine::Behavior::NodeGraph::BehaviorNode)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Behavior::NodeGraph::BehaviorNode, Engine::Behavior::NodeGraph::NodeBase)
    FIELD(mDefaultParameters)
SERIALIZETABLE_END(Engine::Behavior::NodeGraph::BehaviorNode)

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        struct BehaviorInterpretData : NodeInterpreterData, Execution::VirtualState<BehaviorReceiver, BehaviorInterpretData &> {

            static std::vector<Behavior> buildSubBehaviors(uint32_t count, const NodeInterpretHandle<BehaviorNode> &handle)
            {
                std::vector<Behavior> result;
                for (uint32_t i = 0; i < count; ++i) {
                    result.push_back(NodeSender<1> { i } | NodeReceiverWrapper { handle });
                }
                return result;
            }

            BehaviorInterpretData(BehaviorHandle type)
                : Execution::VirtualState<BehaviorReceiver, BehaviorInterpretData &>(*this)
                , mType(type)
            {
            }

            void start(NodeReceiver<NodeBase> receiver, const Reflect::ArgumentList &args)
            {
                NodeInterpretHandle<BehaviorNode> handle { { receiver.mInterpreter }, static_cast<const BehaviorNode &>(receiver.mNode) };

                construct(mState,
                    std::move(receiver),
                    mType.create(args, buildSubBehaviors(mType.descriptor().subBehaviorCount(), handle)).connect(*this));

                mState->mBehavior->start();
            }

            void set_value(Reflect::ArgumentList values)
            {
                mResult = std::move(values);
                NodeReceiver<NodeBase> receiver = std::move(mState->mReceiver);
                destruct(mState);
                receiver.set_value();
            }

            void set_error(Reflect::Error result)
            {
                NodeReceiver<NodeBase> receiver = std::move(mState->mReceiver);
                destruct(mState);
                receiver.set_error(std::move(result));
            }

            void set_done()
            {
                NodeReceiver<NodeBase> receiver = std::move(mState->mReceiver);
                destruct(mState);
                receiver.set_done();
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, BehaviorInterpretData &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, NodeReceiver<NodeBase> &, Args...>
            {
                return f(rec.mState->mReceiver, std::forward<Args>(args)...);
            }

            friend Debug::SenderLocation *tag_invoke(Execution::get_debug_location_t, BehaviorInterpretData &rec)
            {
                return &rec.mState->mLocation;
            }

            BehaviorHandle mType;
            Reflect::ArgumentList mResult;

            struct state {

                state(NodeReceiver<NodeBase> receiver, Behavior::StatePtr behavior)
                    : mReceiver(std::move(receiver))
                    , mBehavior(std::move(behavior))
                    , mLocation([this](Debug::SenderLocation::CB visitor) -> void { mBehavior->visitState(visitor); })
                {
                    receiver.mDebugLocation.mChild = &mLocation;
                }

                ~state()
                {
                    mReceiver.mDebugLocation.mChild = nullptr;
                }

                NodeReceiver<NodeBase> mReceiver;
                Behavior::StatePtr mBehavior;
                Debug::SenderLocation mLocation;
            };
            ManualLifetime<state> mState;
        };

        BehaviorNode::BehaviorNode(NodeGraph &graph, BehaviorHandle behavior)
            : VirtualData(graph)
            , mBehavior(std::move(behavior))
            , mFullClassName(mBehavior.toString())
            , mDefaultParameters(mBehavior.createParameters())
        {
            this->refresh();
        }

        BehaviorNode::BehaviorNode(const BehaviorNode &other, NodeGraph &graph)
            : VirtualData(other, graph)
            , mBehavior(other.mBehavior)
            , mFullClassName(other.mFullClassName)
            , mDefaultParameters(other.mDefaultParameters)
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
                return mBehavior.descriptor().subBehaviorCount();
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
                return mDefaultParameters.size();
            }
        }

        std::string_view BehaviorNode::dataInName(uint32_t index, uint32_t group) const
        {
            if (group == 0) {
                throw 0;
            } else {
                if (index >= mBehavior.descriptor().parameterCount())
                    return "<unknown>";
                return mBehavior.descriptor().parameterName(index);
            }
        }

        Reflect::ExtendedType BehaviorNode::dataInType(uint32_t index, uint32_t group, bool bidir) const
        {
            if (group == 0) {
                throw 0;
            } else {
                if (index >= mBehavior.descriptor().parameterCount())
                    return { Reflect::ExtendedTypeEnum::GenericType };
                return mBehavior.descriptor().parameterType(index);
            }
        }

        uint32_t BehaviorNode::dataOutBaseCount(uint32_t group) const
        {
            return mBehavior.descriptor().resultCount();
        }

        Reflect::ExtendedType BehaviorNode::dataOutType(uint32_t index, uint32_t group, bool bidir) const
        {
            return mBehavior.descriptor().resultType(index);
        }

        void BehaviorNode::setupInterpret(NodeInterpreterStateBase &interpreter, std::unique_ptr<NodeInterpreterData> &data) const
        {
            data = std::make_unique<BehaviorInterpretData>(mBehavior);
        }

        void BehaviorNode::interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const
        {
            static_cast<BehaviorInterpretData *>(data.get())->start(std::move(receiver), mDefaultParameters);
        }

        Reflect::Result BehaviorNode::interpretRead(NodeInterpreterStateBase &interpreter, Reflect::Value &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const
        {
            return static_cast<BehaviorInterpretData *>(data.get())->mRec.mResult.get(retVal, providerIndex);
        }

    }
}
}