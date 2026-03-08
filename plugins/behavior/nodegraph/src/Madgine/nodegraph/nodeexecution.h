#pragma once

#include "Generic/execution/algorithm.h"
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

            KeyValueResult read(const NodeBase &node, ValueType &retVal, uint32_t dataInIndex, uint32_t group = 0);
        };

        template <typename Node>
        struct NodeInterpretHandle : NodeInterpretHandleBase {
            const Node &mNode;

            KeyValueResult read(ValueType &retVal, uint32_t dataInIndex, uint32_t group = 0)
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
            void set_error(KeyValueError result)
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
                throw 0;
            }

            void set_value(ArgumentList args) override
            {
                VirtualBehaviorState<Rec>::set_value(std::move(args));
            }

            void set_error(KeyValueError error) override
            {
                VirtualBehaviorState<Rec>::set_error(std::move(error));
            }

            void set_done() override
            {
                VirtualBehaviorState<Rec>::set_done();
            }

            uint32_t mFlowOutIndex = 0;
            NodeDebugLocation mDebugLocation = &Execution::get_context(this->mRec).mInterpreter;
        };

        template <uint32_t flowOutGroup>
        struct NodeSender {
            using is_sender = void;

            using result_type = KeyValueError;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<ArgumentList>;

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

            using result_type = KeyValueError;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<decayed_t<T>...>;

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
                template <size_t... I>
                void helper(std::index_sequence<I...>)
                {
                    auto &handle = Execution::get_context(this->mRec);
                    if (handle.mNode.dataInCount() == mBaseIndex) {
                        this->set_done();
                    } else {
                        ArgumentList data { sizeof...(T) };
                        for (size_t index = 0; index < sizeof...(T); ++index) {
                            KeyValueResult error = handle.read(data[index], index + mBaseIndex);
                            if (error.mState != GenericResult::SUCCESS) {
                                this->set_error(std::move(*error.mError));
                                return;
                            }
                        }

                        this->set_value(ValueType_as<decayed_t<T>>(data.at(I))...);
                    }
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
                mResults[flowOutGroup] = ArgumentList { std::forward<Args>(args)... };
                return NodeSender<flowOutGroup> {};
            }
            std::vector<NodeResults> &mResults;
        };

        template <uint32_t flowOutGroup>
        struct NodeAlgorithm {

            using Signature = Execution::signature<>;

            template <typename Rec>
            struct receiver : NodeState<flowOutGroup, Rec> {

                receiver(Rec &&rec, std::vector<NodeResults> &results)
                    : NodeState<flowOutGroup, Rec>(std::forward<Rec>(rec), 0)
                    , mResults(results)
                {
                }

                template <typename... Args>
                void set_value(Args &&...args)
                {
                    if (mResults.size() <= flowOutGroup)
                        mResults.resize(flowOutGroup + 1);
                    mResults[flowOutGroup] = { std::forward<Args>(args)... };
                    this->start();
                }

                void set_error(KeyValueError result)
                {
                    this->mRec.set_error(std::move(result));
                }

                void set_done()
                {
                    this->mRec.set_done();
                }

                std::vector<NodeResults> &mResults;
            };

            template <typename Inner>
            struct sender : Execution::base_sender {
                using result_type = void;
                template <template <typename...> typename Tuple>
                using value_types = Tuple<>;

                template <typename Rec>
                friend auto tag_invoke(Execution::connect_t connect, sender &&sender, Rec &&rec)
                {
                    return tag_invoke(connect, std::forward<Inner>(sender.mInner), receiver<Rec> { std::forward<Rec>(rec), sender.mResults });
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