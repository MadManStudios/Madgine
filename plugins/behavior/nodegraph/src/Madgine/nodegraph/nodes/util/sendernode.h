#pragma once

#include "Generic/delayedconstruct.h"
#include "Generic/execution/algorithm.h"
#include "Generic/execution/concepts.h"
#include "Generic/execution/execution.h"
#include "Generic/manuallifetime.h"

#include "Meta/reflect/value.h"

#include "../../nodecollector.h"
#include "../../nodeexecution.h"
#include "../../nodegraph.h"
#include "../../nodeinterpreter.h"
#include "../../pins.h"
#include "automasknode.h"

namespace Engine {
namespace Behavior {
    namespace NodeGraph {

        template <typename T>
        using is_router = Concepts::is_instance_auto1<meta_decayed_t<T>, NodeRouter>;

        template <typename T>
        using is_algorithm = Concepts::is_instance_auto1<meta_decayed_t<T>, NodeAlgorithm>;

        template <typename T>
        using is_pred_sender = std::bool_constant<Concepts::is_instance<meta_decayed_t<T>, NodeReader>::value || Concepts::is_instance<meta_decayed_t<T>, NodeStream>::value>;

        template <typename T>
        using is_succ_sender = Concepts::is_instance_auto1<meta_decayed_t<T>, NodeSender>;

        template <typename T>
        using is_range = Concepts::is_instance_auto1<meta_decayed_t<T>, NodeRange>;

        template <typename T>
        using is_value = std::negation<std::disjunction<is_algorithm<T>, is_pred_sender<T>, is_succ_sender<T>, is_algorithm<T>, is_router<T>, is_range<T>>>;

        template <typename T>
        using is_any_algorithm = std::disjunction<is_algorithm<T>, is_router<T>>;

        template <typename T, size_t>
        struct remove_deductors_impl {
            using type = T;
        };

        template <size_t I>
        struct dynamic_value_type : Reflect::Value {
            static constexpr size_t index = I;
            using meta_t = Value;
            using Value::operator=;
            using no_value_type = void;
        };

        template <size_t I>
        struct remove_deductors_impl<Reflect::Value, I> {
            using type = /*dynamic_value_type<I>*/ Reflect::Value;
        };

        template <typename T, size_t I>
        using remove_deductors = typename remove_deductors_impl<T, I>::type;

        template <typename T, typename Config, auto Algorithm, typename... Arguments>
        struct SenderNode : Node<T, AutoMaskNode<>> {

            using meta_t = T;

            using argument_types = type_pack<Arguments...>;
            using algorithms = typename argument_types::template filter<is_any_algorithm>;
            using value_arguments = typename argument_types::template filter<is_value>;
            using value_argument_tuple = typename value_arguments::template transform_with_index<remove_deductors>::template instantiate<std::tuple>;
            using succ_senders = typename argument_types::template filter<is_succ_sender>;
            using ranges = typename argument_types::template filter<is_range>;
            using in_types = typename argument_types::template filter<is_pred_sender>;

            static constexpr size_t variadicSuccCount = succ_senders::template filter<Execution::is_stream>::size;

            template <uint32_t I>
            static auto buildArgs(const NodeBase &node, const std::tuple<> &values, type_pack<> args, std::vector<NodeResults> *results = nullptr)
            {
                return std::make_tuple();
            }

            template <uint32_t I, typename... Vs, typename Ty, typename... Ts>
            static auto buildArgs(const NodeBase &node, std::tuple<Vs...> &&values, type_pack<Ty, Ts...> args, std::vector<NodeResults> *results = nullptr)
            {
                if constexpr (is_range<Ty>::value) {
                    return std::tuple_cat(
                        std::make_tuple(Ty { node.flowOutCount(is_range<Ty>::value) }),
                        buildArgs<I>(node, std::move(values), type_pack<Ts...> {}, results));
                } else if constexpr (is_pred_sender<Ty>::value) {
                    return std::tuple_cat(
                        std::make_tuple(Ty {}),
                        buildArgs<I>(node, std::move(values), type_pack<Ts...> {}, results));
                } else if constexpr (is_succ_sender<Ty>::value) {
                    return std::tuple_cat(
                        std::make_tuple(NodeSender<I + 1> {}),
                        buildArgs<I + 1>(node, std::move(values), type_pack<Ts...> {}, results));
                } else if constexpr (is_router<Ty>::value) {
                    assert(results);
                    return std::tuple_cat(
                        std::make_tuple(NodeRouter<I + 1> { *results }),
                        buildArgs<I + 1>(node, std::move(values), type_pack<Ts...> {}, results));
                } else if constexpr (is_algorithm<Ty>::value) {
                    assert(results);
                    return std::tuple_cat(
                        std::make_tuple(NodeAlgorithm<I + 1> { *results }),
                        buildArgs<I + 1>(node, std::move(values), type_pack<Ts...> {}, results));
                } else {
                    return TupleUnpacker::prepend<meta_decayed_t<first_t<Vs...>>>(
                        std::get<0>(std::move(values)),
                        buildArgs<I>(node, TupleUnpacker::popFront(std::move(values)), type_pack<Ts...> {}, results));
                }
            }

            static auto buildSender(const NodeBase &node, value_argument_tuple &&values, std::vector<NodeResults> *results = nullptr)
            {
                return TupleUnpacker::invokeFromTuple(Algorithm, buildArgs<0>(node, std::move(values), argument_types {}, results));
            }

            template <typename Ty>
            Reflect::ExtendedType resolveType() const
            {
                using decayedT = std::decay_t<Ty>;
                if constexpr (Concepts::InstanceOfA<decayedT, dynamic_value_type>) {
                    return getArguments<Ty::index>().type();
                } else {
                    return Reflect::toType<std::remove_reference_t<meta_decayed_t<Ty>>>();
                }
            }

            template <typename Signature>
            Reflect::ExtendedType signature_type(uint32_t index) const
            {
                if constexpr (Signature::size == 0) {
                    throw 0;
                } else {
                    return [this, index]<typename... Ty>(type_pack<Ty...>) {
                        Reflect::ExtendedType types[] = { resolveType<Ty>()... };
                        return types[index];
                    }(Signature {});
                }
            }

            template <typename Signature>
            Reflect::ExtendedType stream_type(uint32_t index) const
            {
                return [this, index]<typename... Ty>(type_pack<Ty...>) {
                    Reflect::ExtendedType types[] = {
                        resolveType<Ty>()...
                    };
                    return types[index % Signature::size];
                }(Signature {});
            }

            using Sender = decltype(buildSender(std::declval<NodeBase>(), std::declval<value_argument_tuple>()));

            struct DummyReceiver : NodeExecutionReceiver<T> {
                template <typename... Args>
                void set_value(Args &&...args)
                {
                    mStorage.set_value(std::forward<Args>(args)...);
                }

                template <typename... Args>
                void set_error(Args &&...args)
                {
                    mStorage.set_error(std::forward<Args>(args)...);
                }

                void set_done()
                {
                    mStorage.set_done();
                }

                Execution::ResultStorage<Sender> mStorage;

                template <typename CPO, typename... Args>
                friend auto tag_invoke(CPO f, DummyReceiver &receiver, Args &&...args)
                    -> tag_invoke_result_t<CPO, NodeExecutionReceiver<T> &, Args...>
                {
                    return f(static_cast<NodeExecutionReceiver<T> &>(receiver), std::forward<Args>(args)...);
                }
            };

            SenderNode(NodeGraph &graph)
                : Node<T, AutoMaskNode<>>(graph)
            {
                this->refresh();
            }

            SenderNode(const SenderNode &other, NodeGraph &graph)
                : Node<T, AutoMaskNode<>>(other, graph)
                , mArguments(other.mArguments)
            {
            }

            uint32_t flowInCount(uint32_t group) const override
            {
                return !Config::constant;
            }

            std::string_view flowInName(uint32_t index, uint32_t group) const override
            {
                return "in";
            }

            uint32_t flowOutGroupCount() const override
            {
                constexpr uint32_t algorithm_count = argument_types::template filter<is_algorithm>::size;
                constexpr uint32_t succ_sender_count = succ_senders::size;
                constexpr uint32_t ranges_count = ranges::size;

                return 1 + algorithm_count + succ_sender_count + ranges_count;
            }

            uint32_t flowOutBaseCount(uint32_t group) const override
            {
                static constexpr auto counts = []<typename... InnerAlg, typename... SuccSender, typename... Ranges>(type_pack<InnerAlg...>, type_pack<SuccSender...>, type_pack<Ranges...>) {
                    return std::array {
                        static_cast<int>(!Config::constant),
                        ((void)sizeof(type_pack<InnerAlg>), 1)...,
                        ((void)sizeof(type_pack<SuccSender>), 1)...,
                        ((void)sizeof(type_pack<Ranges>), 1)...
                    };
                }(algorithms {}, succ_senders {}, ranges {});
                return counts[group];
            }

            std::string_view flowOutName(uint32_t index, uint32_t group) const override
            {
                return "out";
            }

            bool flowOutVariadic(uint32_t group = 0) const override
            {
                static constexpr auto variadics = []<typename... InnerAlg, typename... SuccSender, typename... Ranges>(type_pack<InnerAlg...>, type_pack<SuccSender...>, type_pack<Ranges...>) {
                    return std::array {
                        false,
                        ((void)sizeof(type_pack<InnerAlg>), false)...,
                        ((void)sizeof(type_pack<SuccSender>), false)...,
                        ((void)sizeof(type_pack<Ranges>), true)...
                    };
                }(algorithms {}, succ_senders {}, ranges {});
                return variadics[group];
            }

            uint32_t dataInGroupCount() const override
            {
                return in_types::size;
            }

            template <typename Inner>
            static constexpr auto dataInBaseCountHelper()
            {
                if constexpr (Concepts::InstanceOf<Inner, NodeReader>) {
                    return Inner::Signature::size;
                } else {
                    return 0;
                }
            };

            uint32_t dataInBaseCount(uint32_t group) const override
            {
                static constexpr auto sizes = []<typename... Inner>(type_pack<Inner...>) {
                    return std::array<uint32_t, in_types::size> {
                        dataInBaseCountHelper<Inner>()...
                    };
                }(in_types {});

                return sizes[group];
            }

            template <typename Inner>
            static constexpr auto dataInTypeHelper()
            {
                if constexpr (Concepts::InstanceOf<Inner, NodeReader>) {
                    return &SenderNode::signature_type<typename Inner::Signature>;
                } else {
                    return &SenderNode::stream_type<typename Inner::Signature>;
                }
            };

            Reflect::ExtendedType dataInType(uint32_t index, uint32_t group, bool bidir = true) const override
            {
                static constexpr auto types = []<typename... Inner>(type_pack<Inner...>) {
                    return std::array<Reflect::ExtendedType (SenderNode::*)(uint32_t) const, in_types::size> {
                        dataInTypeHelper<Inner>()...
                    };
                }(in_types {});
                return (this->*types[group])(index);
            }

            bool dataInVariadic(uint32_t group = 0) const override
            {
                static constexpr auto variadic = []<typename... Inner>(type_pack<Inner...>) {
                    return std::array<bool, in_types::size> {
                        Concepts::InstanceOf<Inner, NodeStream>...
                    };
                }(in_types {});

                if (group >= variadic.size())
                    return false;

                return variadic[group];
            }

            uint32_t dataOutGroupCount() const override
            {
                return 1 + algorithms::size;
            }

            uint32_t dataOutBaseCount(uint32_t group) const override
            {
                static constexpr auto sizes = []<typename... InnerAlg>(type_pack<InnerAlg...>) {
                    return std::array {
                        Sender::template value_types<type_pack>::size,
                        InnerAlg::Signature::size...
                    };
                }(algorithms {});
                return sizes[group];
            }

            Reflect::ExtendedType dataOutType(uint32_t index, uint32_t group, bool bidir = true) const override
            {
                static constexpr auto types = []<typename... InnerAlg>(type_pack<InnerAlg...>) {
                    return std::array {
                        &SenderNode::template signature_type<typename Sender::template value_types<Execution::signature>>,
                        &SenderNode::template signature_type<typename InnerAlg::Signature>...
                    };
                }(algorithms {});
                return (this->*types[group])(index);
            }

            struct InterpretData : NodeInterpreterData {

                struct Receiver {
                    InterpretData *mData;
                    NodeReceiver<T> mReceiver;

                    template <typename... Args>
                    void set_value(Args &&...args)
                    {
                        if (mData->mResults.empty())
                            mData->mResults.emplace_back();
                        mData->mResults.front() = Reflect::ArgumentList { std::forward<Args>(args)... };
                        NodeReceiver<T> rec = std::move(mReceiver);
                        mData->cleanup();
                        rec.set_value();
                    }

                    void set_done()
                    {
                        NodeReceiver<T> rec = std::move(mReceiver);
                        mData->cleanup();
                        rec.set_done();
                    }

                    void set_error(Reflect::Error result)
                    {
                        NodeReceiver<T> rec = std::move(mReceiver);
                        mData->cleanup();
                        rec.set_error(std::move(result));
                    }

                    template <typename CPO, typename... Args>
                    friend auto tag_invoke(CPO f, Receiver &receiver, Args &&...args)
                        -> tag_invoke_result_t<CPO, NodeReceiver<T> &, Args...>
                    {
                        return f(receiver.mReceiver, std::forward<Args>(args)...);
                    }
                };

                using State = Execution::connect_result_t<Execution::with_debug_location_t::sender<Execution::stoppable_t::sender<Sender>>, Receiver>;

                InterpretData()
                {
                }

                ~InterpretData()
                {
                }

                void start(NodeReceiver<T> receiver, value_argument_tuple args)
                {
                    const NodeBase &node = Execution::get_context(receiver).mNode;
                    construct(mState,
                        DelayedConstruct<State> { [&]() { return Execution::connect(buildSender(node, std::move(args), &mResults) | Execution::stoppable | Execution::with_debug_location(receiver.mDebugLocation.mChild), Receiver { this, std::move(receiver) }); } });
                    mState->start();
                }

                void cleanup()
                {
                    destruct(mState);
                }

                Reflect::Result read(Reflect::Value &retVal, uint32_t providerIndex, uint32_t group) const
                {
                    return mResults[group].get(retVal, providerIndex);
                }

                std::vector<NodeResults> mResults;
                ManualLifetime<State> mState;
            };

            void interpret(NodeReceiver<NodeBase> receiver, std::unique_ptr<NodeInterpreterData> &data, uint32_t flowIn, uint32_t group) const override
            {
                if constexpr (!Config::constant) {
                    if (!data) {
                        data = std::make_unique<InterpretData>();
                    }
                    static_cast<InterpretData *>(data.get())->start({ { { { receiver.mInterpreter }, static_cast<const T &>(receiver.mNode) } }, receiver.mReceiver, receiver.mDebugLocation }, mArguments);
                } else {
                    throw 0;
                }
            }

            Reflect::Result interpretRead(NodeInterpreterStateBase &interpreter, Reflect::Value &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const override
            {
                if constexpr (Config::constant) {

                    DummyReceiver rec { { { { interpreter }, static_cast<const T &>(*this) } } };

                    auto state = Execution::connect(buildSender(*this, value_argument_tuple { mArguments }), rec);

                    state.start();

                    assert(!rec.mStorage.is_null());
                    if constexpr (rec.mStorage.can_have_error) {
                        if (rec.mStorage.is_error()) {
                            return std::make_unique<Reflect::Error>(std::move(rec.mStorage).error().mError);
                        }
                    }
                    retVal = TupleUnpacker::select(
                        std::move(rec.mStorage).value().mValues,
                        [](auto &&v) -> Reflect::Value {
                            return Reflect::Value { std::forward<decltype(v)>(v) };
                        },
                        providerIndex);
                } else {
                    assert(data);
                    return static_cast<InterpretData *>(data.get())->read(retVal, providerIndex, group);
                }
                return {};
            }

            value_argument_tuple mArguments;
            template <size_t I>
            const meta_decayed_t<std::tuple_element_t<I, value_argument_tuple>> &getArguments() const
            {
                return std::get<I>(mArguments);
            }
            template <size_t I>
            void setArguments(meta_decayed_t<std::tuple_element_t<I, value_argument_tuple>> v)
            {
                std::get<I>(mArguments) = v;
            }

            template <fixed_string Name>
            struct NamedString {
                std::string mString;
            };
        };
    }
}
}