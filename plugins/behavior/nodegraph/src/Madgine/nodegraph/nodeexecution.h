#pragma once

#include "Generic/execution/concepts.h"
#include "Madgine/bindings.h"

#include "Generic/execution/algorithm.h"
#include "Generic/execution/execution.h"

#include "Meta/keyvalue/valuetype.h"

#include "nodebase.h"
#include "nodeinterpreter.h"

#include "Madgine/codegen/fromsender.h"

namespace Engine {
namespace NodeGraph {

    struct MADGINE_NODEGRAPH_EXPORT NodeInterpretHandleBase {
        NodeInterpreterStateBase &mInterpreter;

        BehaviorError read(const NodeBase &node, ValueType &retVal, uint32_t dataInIndex, uint32_t group = 0);

        void write(const NodeBase &node, const ValueType &v, uint32_t dataOutIndex, uint32_t group = 0);

        BehaviorError getBinding(std::string_view name, ValueType &out);
    };

    template <typename Node>
    struct NodeInterpretHandle : NodeInterpretHandleBase {
        const Node &mNode;

        BehaviorError read(ValueType &retVal, uint32_t dataInIndex, uint32_t group = 0)
        {
            return NodeInterpretHandleBase::read(mNode, retVal, dataInIndex, group);
        }

        void write(const ValueType &v, uint32_t dataOutIndex, uint32_t group = 0)
        {
            NodeInterpretHandleBase::write(mNode, v, dataOutIndex, group);
        }

        template <fixed_string Name, typename O>
        friend BehaviorError tag_invoke(get_binding_t<Name>, NodeInterpretHandle &handle, O &out)
        {
            ValueType v;
            BehaviorError error = handle.getBinding(handle.mNode.template getDynamicName<Name>(), v))
            if (error.mResult == BehaviorResult{BehaviorResult::SUCCESS})
                out = v.as<O>();
            return error;
        }
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
        void set_error(BehaviorError result)
        {
            mReceiver.set_error(result);
        }

        friend NodeDebugLocation *tag_invoke(Execution::get_debug_location_t, NodeReceiver &rec)
        {
            return &rec.mDebugLocation;
        }

        template <typename CPO, typename... Args>
        friend auto tag_invoke(CPO f, NodeReceiver &rec, Args &&...args)
            -> tag_invoke_result_t<CPO, BehaviorReceiver &, Args...>
        {
            return f(rec.mReceiver, std::forward<Args>(args)...);
        }
    };

    struct MADGINE_NODEGRAPH_EXPORT NodeCodegenHandle : CodeGen::CodeGen_Context {
        const NodeBase *mNode;
        CodeGenerator &mGenerator;

        CodeGen::Statement read(uint32_t dataInIndex, uint32_t group = 0);
    };

    using NodeCodegenReceiver = Execution::execution_receiver<NodeCodegenHandle>;

    template <uint32_t flowOutIndex, typename Rec>
    struct NodeState : VirtualBehaviorState<Rec> {

        using VirtualBehaviorState<Rec>::VirtualBehaviorState;

        void start()
        {
            mDebugLocation.stepInto(Execution::get_debug_location(this->mRec));
            auto &handle = Execution::get_context(this->mRec);
            handle.mInterpreter.branch(*this, handle.mNode.flowOutTarget(0, flowOutIndex), mDebugLocation);
        }

        void set_value(ArgumentList args) override
        {
            mDebugLocation.stepOut(Execution::get_debug_location(this->mRec));
            VirtualBehaviorState<Rec>::set_value(std::move(args));
        }

        void set_error(BehaviorError error) override
        {
            mDebugLocation.stepOut(Execution::get_debug_location(this->mRec));
            VirtualBehaviorState<Rec>::set_error(std::move(error));
        }

        void set_done() override
        {
            mDebugLocation.stepOut(Execution::get_debug_location(this->mRec));
            VirtualBehaviorState<Rec>::set_done();
        }

        NodeDebugLocation mDebugLocation = &Execution::get_context(this->mRec).mInterpreter;
    };

    template <uint32_t flowOutIndex>
    struct NodeSender {
        using result_type = BehaviorError;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, NodeSender &&sender, Rec &&rec)
        {
            return NodeState<flowOutIndex, Rec> { std::forward<Rec>(rec) };
        }

        template <typename Rec>
        friend auto tag_invoke(CodeGen::codegen_connect_t, NodeSender &&sender, Rec &&rec)
        {
            struct state : CodeGen::codegen_base_state<Rec> {
                auto generate()
                {
                    return std::make_tuple(CodeGen::Constant { ValueType { 0 } });
                }
            };
            return state { std::forward<Rec>(rec) };
        }
    };

    template <typename... T>
    struct NodeReader {
        using Signature = Execution::signature<T...>;

        using result_type = BehaviorError;
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

            template <typename Ty>
            struct typed_Value : ValueType {
                using ValueType::operator=;
            };
            void start()
            {
                helper(std::index_sequence_for<T...> {});
            }
            template <size_t... I>
            void helper(std::index_sequence<I...>)
            {
                auto &handle = Execution::get_context(this->mRec);
                if (handle.mNode.dataInCount() == mIndex) {
                    this->set_done();
                } else {
                    assert(mIndex == 0);
                    std::tuple<typed_Value<T>...> data;
                    BehaviorError error = TupleUnpacker::accumulate(
                        data, [&]<typename Ty>(typed_Value<Ty> &v, BehaviorError e) {
                            if (e.mResult != GenericResult::SUCCESS)
                                return e;
                            return handle.read(v, mIndex++);
                        },
                        BehaviorError {});
                    mIndex = 0;
                    if (error.mResult != GenericResult::SUCCESS) {
                        this->set_error(std::move(error));
                    } else {
                        this->set_value(std::get<I>(data).template as<decayed_t<T>>()...);
                    }
                }
            }

            size_t mIndex = 0;
        };

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, NodeReader &&reader, Rec &&rec)
        {
            return state<Rec> { std::forward<Rec>(rec), reader.mBaseIndex };
        }

        template <typename Rec>
        struct codegen_state : CodeGen::codegen_base_state<Rec> {
            auto generate()
            {
                return helper(std::index_sequence_for<T...> {});
            }
            template <size_t... I>
            auto helper(std::index_sequence<I...>)
            {
                return std::make_tuple(Execution::get_context(this->mRec).read(mIndex + I)...);
            }
            size_t mIndex;
        };

        template <typename Rec>
        friend auto tag_invoke(CodeGen::codegen_connect_t, NodeReader &&reader, Rec &&rec)
        {
            return codegen_state<Rec> { std::forward<Rec>(rec), reader.mBaseIndex };
        }

        size_t mBaseIndex = 0;
    };

    template <size_t flowOutIndex, typename... Arguments>
    struct NodeRouter {

        using Signature = Execution::signature<Arguments...>;

        template <typename... Args>
        auto operator()(Args &&...args)
        {
            if (mResults.size() <= flowOutIndex)
                mResults.resize(flowOutIndex + 1);
            mResults[flowOutIndex] = { std::forward<Args>(args)... };
            return NodeSender<flowOutIndex> {};
        }
        std::vector<NodeResults> &mResults;
    };

    template <size_t flowOutIndex>
    struct NodeAlgorithm {

        using Signature = Execution::signature<>;

        template <typename Rec>
        struct receiver : NodeState<flowOutIndex, Rec> {

            receiver(Rec &&rec, std::vector<NodeResults> &results)
                : NodeState<flowOutIndex, Rec>(std::forward<Rec>(rec))
                , mResults(results)
            {
            }

            template <typename... Args>
            void set_value(Args &&...args)
            {
                if (mResults.size() <= flowOutIndex)
                    mResults.resize(flowOutIndex + 1);
                mResults[flowOutIndex] = { std::forward<Args>(args)... };
                this->start();
            }

            void set_error(BehaviorError result)
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
            friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
            {
                return Execution::connect(std::forward<Inner>(sender.mInner), receiver<Rec> { std::forward<Rec>(rec), sender.mResults });
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

}
}