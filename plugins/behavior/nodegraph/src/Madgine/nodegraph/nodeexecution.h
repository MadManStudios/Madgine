#pragma once

#include "Generic/execution/query.h"
#include "Generic/execution/concepts.h"
#include "Generic/execution/execution.h"

#include "Madgine/behavior/named.h"

#include "nodebase.h"
#include "nodeinterpreter.h"

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        struct MADGINE_NODEGRAPH_EXPORT NodeInterpretHandleBase {
            NodeInterpreterStateBase &mInterpreter;

            Reflect::Result read(const NodeBase &node, Reflect::Value &retVal, uint32_t dataInIndex, uint32_t group = 0);
        };

        template <typename Node>
        struct NodeInterpretHandle : NodeInterpretHandleBase {
            const Node &mNode;

            Reflect::Result read(Reflect::Value &retVal, uint32_t dataInIndex, uint32_t group = 0)
            {
                return NodeInterpretHandleBase::read(mNode, retVal, dataInIndex, group);
            }

            /* template <fixed_string Name, typename O>
            friend BehaviorError tag_invoke(get_binding_t<Name>, NodeInterpretHandle &handle, O &out)
            {
                ValueType v;
                BehaviorError error = handle.getBinding(Name, v);
                if (error.mResult == BehaviorResult { BehaviorResult::SUCCESS })
                    out = v.as<O>();
                return error;
            }*/
        };

        template <typename Node>
        using NodeExecutionReceiver = Execution::execution_receiver<NodeInterpretHandle<Node>>;

        template <typename Node>
        struct NodeReceiver : NodeExecutionReceiver<Node> {
            BehaviorReceiver &mReceiver;
            NodeDebugLocation &mDebugLocation;

            void set_value()
            {
                continueExecution(this->mInterpreter, this->mNode, mReceiver, mDebugLocation);
            }
            void set_done()
            {
                mReceiver.set_done();
            }
            void set_error(Reflect::Error result)
            {
                mReceiver.set_error(std::move(result));
            }

            friend NodeDebugLocation *tag_invoke(Execution::get_debug_location_t, NodeReceiver &rec)
            {
                return &rec.mDebugLocation;
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, NodeReceiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, BehaviorReceiver &, Args...>
            {
                return tag_invoke(f, rec.mReceiver, std::forward<Args>(args)...);
            }
        };

        template <typename Node>
        struct NodeReceiverWrapper {

            template <typename Sender>
            struct sender : Execution::algorithm_sender<Sender> {
                template <typename Rec>
                friend auto tag_invoke(Execution::connect_t connect, sender &&sender, Rec &&rec)
                {
                    return tag_invoke(connect, std::forward<Sender>(sender.mSender) | Execution::with_query_value(Execution::get_context, std::move(sender.mHandle)), std::forward<Rec>(rec));
                }

                template <typename Rec>
                friend auto tag_invoke(Execution::connect_t connect, sender &sender, Rec &&rec)
                {
                    return tag_invoke(connect, sender.mSender | Execution::with_query_value(Execution::get_context, sender.mHandle), std::forward<Rec>(rec));
                }

                NodeInterpretHandle<Node> mHandle;
            };

            template <typename Inner>
            auto operator()(Inner &&inner) const
            {
                return sender<Inner> { { {}, std::forward<Inner>(inner) }, std::move(mHandle) };
            }

            template <typename Inner>
            friend auto operator|(Inner &&inner, NodeReceiverWrapper &&wrapper)
            {
                return sender<Inner> { { {}, std::forward<Inner>(inner) }, std::move(wrapper.mHandle) };
            }

            NodeInterpretHandle<Node> mHandle;
        };

        template <typename Node>
        NodeReceiverWrapper(NodeInterpretHandle<Node>) -> NodeReceiverWrapper<Node>;

        template <uint32_t flowOutGroup, typename Rec>
        struct NodeState : VirtualBehaviorState<Rec> {

            NodeState(Rec &&rec, uint32_t flowOutIndex)
                : VirtualBehaviorState<Rec> { std::forward<Rec>(rec) }
                , mFlowOutIndex { flowOutIndex }
            {
            }

            void start()
            {
                const auto &handle = Execution::get_context(this->mRec);
                handle.mInterpreter.branch(*this, handle.mNode.flowOutTarget(mFlowOutIndex, flowOutGroup), mDebugLocation);
            }

            void stop()
            {
                const auto &handle = Execution::get_context(this->mRec);
                handle.mInterpreter.stop();
            }

            friend void tag_invoke(Execution::visit_state_t visit_state, NodeState *s, auto &&visitor)
            {
                visitor(Execution::State::DebugLocation { s ? &s->mDebugLocation : nullptr });
            }

            uint32_t mFlowOutIndex = 0;
            NodeDebugLocation mDebugLocation = &Execution::get_context(this->mRec).mInterpreter;
        };

        template <uint32_t flowOutGroup>
        struct NodeSender {
            using is_sender = void;

            using result_type = Reflect::Error;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<Reflect::ArgumentList>;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, NodeSender &&sender, Rec &&rec)
            {
                return NodeState<flowOutGroup, Rec> { std::forward<Rec>(rec), sender.mFlowOutIndex };
            }

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, NodeSender &sender, Rec &&rec)
            {
                return NodeState<flowOutGroup, Rec> { std::forward<Rec>(rec), sender.mFlowOutIndex };
            }

            uint32_t mFlowOutIndex = 0;
        };

        template <typename... T>
        struct NodeReader {
            using Signature = Execution::signature<T...>;

            using is_sender = void;

            using result_type = Reflect::Error;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<meta_decayed_t<T>...>;

            NodeReader(size_t *baseIndex = nullptr)
                : mBaseIndex(baseIndex ? *baseIndex : 0)
            {
                if (baseIndex)
                    *baseIndex += sizeof...(T);
            }

            template <typename Rec>
            struct state : Execution::base_state<Rec> {

                void start()
                {
                    helper(std::index_sequence_for<T...> {});
                }
                void stop()
                {
                }
                template <size_t... I>
                void helper(std::index_sequence<I...>)
                {
                    auto &handle = Execution::get_context(this->mRec);
                    if (handle.mNode.dataInCount() == mBaseIndex) {
                        this->set_done();
                    } else {
                        Reflect::ArgumentList data { std::true_type {}, sizeof...(T) };
                        for (size_t index = 0; index < sizeof...(T); ++index) {
                            Reflect::Result error = handle.read(data[index], index + mBaseIndex);
                            if (error) {
                                this->set_error(std::move(*error.mError));
                                return;
                            }
                        }

                        Reflect::Result error = invoke_free([this](meta_decayed_t<T>... val) { this->set_value(std::forward<meta_decayed_t<T>>(val)...); }, data.at(I)...);
                        if (error)
                            this->set_error(std::move(*error.mError));
                    }
                }

                friend void tag_invoke(Execution::visit_state_t, state *s, auto &&visitor)
                {
                    //Not helpful to show
                    //visitor(Execution::State::Text { "NodeReader" });
                }

                size_t mBaseIndex;
            };

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, NodeReader &&reader, Rec &&rec)
            {
                return state<Rec> { std::forward<Rec>(rec), reader.mBaseIndex };
            }

            size_t mBaseIndex = 0;
        };

        template <uint32_t flowOutGroup, typename... Arguments>
        struct NodeRouter {

            using Signature = Execution::signature<Arguments...>;

            template <typename... Args>
            auto operator()(Args &&...args)
            {
                if (mResults.size() <= flowOutGroup)
                    mResults.resize(flowOutGroup + 1);
                mResults[flowOutGroup] = Reflect::ArgumentList { std::forward<Args>(args)... };
                return NodeSender<flowOutGroup> {};
            }
            std::vector<NodeResults> &mResults;
        };

        template <uint32_t flowOutGroup>
        struct NodeAlgorithm {

            using Signature = Execution::signature<>;

            template <typename Inner, typename Rec>
            struct state;

            template <typename Inner, typename Rec>
            struct receiver {

                receiver(state<Inner, Rec> *state, std::vector<NodeResults> &results)
                    : mState(state)
                    , mResults(results)
                {
                }

                template <typename... Args>
                void set_value(Args &&...args)
                {
                    if (mResults.size() <= flowOutGroup)
                        mResults.resize(flowOutGroup + 1);
                    mResults[flowOutGroup] = { std::forward<Args>(args)... };
                    mState->startAlgorithm();
                }

                void set_error(Reflect::Error result)
                {
                    mState->set_error(std::move(result));
                }

                void set_done()
                {
                    mState->set_done();
                }

                template <typename CPO, typename... Args>
                    requires(is_tag_invocable_v<CPO, Rec &, Args...>)
                friend auto tag_invoke(CPO f, receiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, Rec &, Args...>)
                    -> tag_invoke_result_t<CPO, Rec &, Args...>
                {
                    return tag_invoke(f, rec.mState->mRec, std::forward<Args>(args)...);
                }

                state<Inner, Rec> *mState;
                std::vector<NodeResults> &mResults;
            };

            template <typename Inner, typename Rec>
            struct state : NodeState<flowOutGroup, Rec> {

                using inner_state = Execution::connect_result_t<Execution::stoppable_t::sender<Inner>, receiver<Inner, Rec>>;

                state(Inner &&inner, Rec &&rec, std::vector<NodeResults> &results)
                    : NodeState<flowOutGroup, Rec>(std::forward<Rec>(rec), 0)
                    , mInnerState(Execution::connect(std::forward<Inner>(inner) | Execution::stoppable, receiver<Inner, Rec> { this, results }))
                {
                }

                void start() {
                    mPassed = false;
                    mInnerState.start();
                }

                void startAlgorithm() {
                    mPassed = true;
                    NodeState<flowOutGroup, Rec>::start();
                }

                friend void tag_invoke(Execution::visit_state_t visit_state, state *s, auto &&visitor)
                {
                    visit_state(!s || s->mPassed ? nullptr : &s->mInnerState, visitor);
                    visit_state(s && s->mPassed ? static_cast<NodeState<flowOutGroup, Rec> *>(s) : nullptr, visitor);                    
                }

                inner_state mInnerState;
                bool mPassed = false;
            };

            template <typename Inner>
            struct sender : Execution::base_sender {
                using result_type = void;
                template <template <typename...> typename Tuple>
                using value_types = Tuple<>;

                template <typename Rec>
                friend auto tag_invoke(Execution::connect_t connect, sender &&sender, Rec &&rec)
                {
                    return state<Inner, Rec> { std::forward<Inner>(sender.mInner), std::forward<Rec>(rec), sender.mResults };
                }

                Inner mInner;
                std::vector<NodeResults> &mResults;
            };

            template <typename Inner>
            auto operator()(Inner &&inner)
            {
                return sender<Inner> { {}, std::forward<Inner>(inner), mResults };
            }
            std::vector<NodeResults> &mResults;
        };

        MADGINE_NODEGRAPH_EXPORT void continueExecution(NodeInterpreterStateBase &interpreter, const NodeBase &node, BehaviorReceiver &receiver, NodeDebugLocation &location);

        template <typename T>
        struct NodeStream {
            using Signature = Execution::signature<T>;

            NodeReader<T> next()
            {
                return { &mIndex };
            }

            size_t mIndex = 0;
        };

        template <uint32_t flowOutGroup>
        struct NodeRange {

            using value_type = NodeSender<flowOutGroup>;

            NodeRange(uint32_t size)
                : mSize(size)
            {
            }

            uint32_t size() const
            {
                return mSize;
            }

            struct iterator {

                constexpr bool operator!=(const iterator &other) const
                {
                    return mIndex != other.mIndex;
                }

                void operator++()
                {
                    ++mIndex;
                }

                auto operator*()
                {
                    return NodeSender<flowOutGroup> { mIndex };
                }

                uint32_t mIndex;
            };

            iterator begin()
            {
                return { 0 };
            }

            iterator end()
            {
                return { mSize };
            }

            uint32_t mSize;
        };

    }
}
}