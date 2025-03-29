#pragma once

#include "Generic/execution/concepts.h"

#include "../../nodecollector.h"
#include "../../nodeinterpreter.h"
#include "Generic/delayedconstruct.h"
#include "Generic/execution/algorithm.h"
#include "Generic/execution/execution.h"
#include "Generic/manuallifetime.h"

#include "../../nodeexecution.h"
#include "../../nodegraph.h"
#include "../../pins.h"
#include "automasknode.h"

#include "Madgine/codegen/fromsender.h"

namespace Engine {

namespace NodeGraph {

    template <typename T>
    using is_router = is_instance_auto1<decayed_t<T>, NodeRouter>;

    template <typename T>
    using is_algorithm = is_instance_auto1<decayed_t<T>, NodeAlgorithm>;

    template <typename T>
    using is_pred_sender = std::bool_constant<is_instance<decayed_t<T>, NodeReader>::value || is_instance<decayed_t<T>, NodeStream>::value>;

    template <typename T>
    using is_succ_sender = is_instance_auto1<decayed_t<T>, NodeSender>;

    template <typename T>
    using is_range = is_instance_auto1<decayed_t<T>, NodeRange>;

    template <typename T>
    using is_value = std::negation<std::disjunction<is_algorithm<T>, is_pred_sender<T>, is_succ_sender<T>, is_algorithm<T>, is_router<T>, is_range<T>>>;

    template <typename T>
    using is_any_algorithm = std::disjunction<is_algorithm<T>, is_router<T>>;

    template <typename T, size_t>
    struct remove_deductors_impl {
        using type = T;
    };

    template <size_t I>
    struct dynamic_value_type : ValueType {
        static constexpr size_t index = I;
        using decay_t = ValueType;
        using ValueType::operator=;
        using no_value_type = void;
    };

    template <size_t I>
    struct remove_deductors_impl<ValueType, I> {
        using type = /*dynamic_value_type<I>*/ ValueType;
    };

    template <typename T, size_t I>
    using remove_deductors = typename remove_deductors_impl<T, I>::type;

    template <typename Config, auto Algorithm, typename... Arguments>
    struct SenderNode : Node<SenderNode<Config, Algorithm, Arguments...>, AutoMaskNode<>> {

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

        template <uint32_t I, typename... Vs, typename T, typename... Ts>
        static auto buildArgs(const NodeBase &node, std::tuple<Vs...> &&values, type_pack<T, Ts...> args, std::vector<NodeResults> *results = nullptr)
        {
            if constexpr (is_range<T>::value) {
                return std::tuple_cat(
                    std::make_tuple(T { node.flowOutCount(is_range<T>::value) }),
                    buildArgs<I>(node, std::move(values), type_pack<Ts...> {}, results));
            } else if constexpr (is_pred_sender<T>::value) {
                return std::tuple_cat(
                    std::make_tuple(T {}),
                    buildArgs<I>(node, std::move(values), type_pack<Ts...> {}, results));
            } else if constexpr (is_succ_sender<T>::value) {
                return std::tuple_cat(
                    std::make_tuple(NodeSender<I + 1> {}),
                    buildArgs<I + 1>(node, std::move(values), type_pack<Ts...> {}, results));
            } else if constexpr (is_router<T>::value) {
                assert(results);
                return std::tuple_cat(
                    std::make_tuple(NodeRouter<I + 1> { *results }),
                    buildArgs<I + 1>(node, std::move(values), type_pack<Ts...> {}, results));
            } else if constexpr (is_algorithm<T>::value) {
                assert(results);
                return std::tuple_cat(
                    std::make_tuple(NodeAlgorithm<I + 1> { *results }),
                    buildArgs<I + 1>(node, std::move(values), type_pack<Ts...> {}, results));
            } else {
                return TupleUnpacker::prepend<decayed_t<first_t<Vs...>>>(
                    std::get<0>(std::move(values)),
                    buildArgs<I>(node, TupleUnpacker::popFront(std::move(values)), type_pack<Ts...> {}, results));
            }
        }

        static auto buildSender(const NodeBase &node, value_argument_tuple &&values, std::vector<NodeResults> *results = nullptr)
        {
            if constexpr (Config::constant)
                return TupleUnpacker::invokeFromTuple(Algorithm, buildArgs<0>(node, std::move(values), argument_types {}, results));
            else
                return TupleUnpacker::invokeFromTuple(Algorithm, buildArgs<0>(node, std::move(values), argument_types {}, results)) | Execution::with_debug_location<Debug::SenderLocation>();
        }

        template <typename T>
        ExtendedValueTypeDesc resolveType() const
        {
            using decayedT = std::decay_t<T>;
            if constexpr (InstanceOfA<decayedT, dynamic_value_type>) {
                return getArguments<T::index>().type();
            } else {
                return toValueTypeDesc<std::remove_reference_t<decayed_t<T>>>();
            }
        }

        template <typename Signature>
        ExtendedValueTypeDesc signature_type(uint32_t index) const
        {
            return [this, index]<typename... T>(type_pack<T...>) {
                ExtendedValueTypeDesc types[] = { resolveType<T>()... };
                return types[index];
            }(Signature {});
        }

        template <typename Signature>
        ExtendedValueTypeDesc stream_type(uint32_t index) const
        {
            return [this, index]<typename... T>(type_pack<T...>) {
                ExtendedValueTypeDesc types[] = {
                    resolveType<T>()...
                };
                return types[index % Signature::size];
            }(Signature {});
        }

        using Sender = decltype(buildSender(std::declval<NodeBase>(), std::declval<value_argument_tuple>()));

        struct DummyReceiver : NodeExecutionReceiver<SenderNode<Config, Algorithm, Arguments...>> {
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
                -> tag_invoke_result_t<CPO, NodeExecutionReceiver<SenderNode<Config, Algorithm, Arguments...>> &, Args...>
            {
                return f(static_cast<NodeExecutionReceiver<SenderNode<Config, Algorithm, Arguments...>> &>(receiver), std::forward<Args>(args)...);
            }
        };

        SenderNode(NodeGraph &graph)
            : Node<SenderNode<Config, Algorithm, Arguments...>, AutoMaskNode<>>(graph)
        {
            this->setup();
        }

        SenderNode(const SenderNode &other, NodeGraph &graph)
            : Node<SenderNode<Config, Algorithm, Arguments...>, AutoMaskNode<>>(other, graph)
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
            if constexpr (InstanceOf<Inner, NodeReader>) {
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
            if constexpr (InstanceOf<Inner, NodeReader>) {
                return &SenderNode::signature_type<typename Inner::Signature>;
            } else {
                return &SenderNode::stream_type<typename Inner::Signature>;
            }
        };

        ExtendedValueTypeDesc dataInType(uint32_t index, uint32_t group, bool bidir = true) const override
        {
            static constexpr auto types = []<typename... Inner>(type_pack<Inner...>) {
                return std::array<ExtendedValueTypeDesc (SenderNode::*)(uint32_t) const, in_types::size> {
                    dataInTypeHelper<Inner>()...
                };
            }(in_types {});
            return (this->*types[group])(index);
        }

        bool dataInVariadic(uint32_t group = 0) const override
        {
            static constexpr auto variadic = []<typename... Inner>(type_pack<Inner...>) {
                return std::array<bool, in_types::size> {
                    InstanceOf<Inner, NodeStream>...
                };
            }(in_types {});

            if (group >= variadic.size())
                return false;

            return variadic[group];
        }

        uint32_t dataProviderGroupCount() const override
        {
            return 1 + algorithms::size;
        }

        uint32_t dataProviderBaseCount(uint32_t group) const override
        {
            static constexpr auto sizes = []<typename... InnerAlg>(type_pack<InnerAlg...>) {
                return std::array {
                    Sender::template value_types<type_pack>::size,
                    InnerAlg::Signature::size...
                };
            }(algorithms {});
            return sizes[group];
        }

        ExtendedValueTypeDesc dataProviderType(uint32_t index, uint32_t group, bool bidir = true) const override
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
                NodeReceiver<SenderNode<Config, Algorithm, Arguments...>> mReceiver;

                template <typename... Args>
                void set_value(Args &&...args)
                {
                    if (mData->mResults.empty())
                        mData->mResults.emplace_back();
                    mData->mResults.front() = { std::forward<Args>(args)... };
                    NodeReceiver<SenderNode<Config, Algorithm, Arguments...>> rec = std::move(mReceiver);
                    mData->cleanup();
                    rec.set_value();
                }

                void set_done()
                {
                    NodeReceiver<SenderNode<Config, Algorithm, Arguments...>> rec = std::move(mReceiver);
                    mData->cleanup();
                    rec.set_done();
                }

                void set_error(BehaviorError result)
                {
                    NodeReceiver<SenderNode<Config, Algorithm, Arguments...>> rec = std::move(mReceiver);
                    mData->cleanup();
                    rec.set_error(result);
                }

                template <typename CPO, typename... Args>
                friend auto tag_invoke(CPO f, Receiver &receiver, Args &&...args)
                    -> tag_invoke_result_t<CPO, NodeReceiver<SenderNode<Config, Algorithm, Arguments...>> &, Args...>
                {
                    return f(receiver.mReceiver, std::forward<Args>(args)...);
                }
            };

            using State = Execution::connect_result_t<Sender, Receiver>;

            InterpretData()
            {
            }

            ~InterpretData()
            {
            }

            void start(NodeReceiver<SenderNode<Config, Algorithm, Arguments...>> receiver, value_argument_tuple args)
            {               
                const NodeBase &node = Execution::get_context(receiver).mNode;
                construct(mState,
                    DelayedConstruct<State> { [&]() { return Execution::connect(buildSender(node, std::move(args), &mResults), Receiver { this, std::move(receiver) }); } });
                mState->start();
            }

            void cleanup()
            {
                destruct(mState);
            }

            ValueType read(uint32_t providerIndex, uint32_t group) const
            {
                return mResults[group][providerIndex];
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
                static_cast<InterpretData *>(data.get())->start({ { receiver.mInterpreter, static_cast<const SenderNode<Config, Algorithm, Arguments...> &>(receiver.mNode) }, receiver.mReceiver, receiver.mDebugLocation }, mArguments);
            } else {
                throw 0;
            }
        }

        BehaviorError interpretRead(NodeInterpreterStateBase &interpreter, ValueType &retVal, std::unique_ptr<NodeInterpreterData> &data, uint32_t providerIndex, uint32_t group) const override
        {
            if constexpr (Config::constant) {

                DummyReceiver rec { interpreter, *this };

                auto state = Execution::connect(buildSender(*this, value_argument_tuple { mArguments }), rec);

                state.start();

                assert(!rec.mStorage.is_null());
                if constexpr (rec.mStorage.can_have_error) {
                    if (rec.mStorage.is_error()) {
                        return std::move(rec.mStorage).error().mError;
                    }
                }
                retVal = TupleUnpacker::select(
                    std::move(rec.mStorage).value().mValues,
                    [](auto &&v) -> ValueType {
                        return ValueType { std::forward<decltype(v)>(v) };
                    },
                    providerIndex);
            } else {
                assert(data);
                retVal = static_cast<InterpretData *>(data.get())->read(providerIndex, group);
            }
            return {};
        }

        virtual CodeGen::Statement generateRead(CodeGenerator &generator, std::unique_ptr<CodeGeneratorData> &data, uint32_t providerIndex, uint32_t group) const override
        {

            /* auto result = CodeGen::generatorFromSender(buildSender(value_argument_tuple { mArguments }), NodeCodegenReceiver { 0, this, generator }).generate();
            if constexpr (std::tuple_size_v<decltype(result)> < 1)
                throw 0;
            else {
                CodeGen::Statement current = TupleUnpacker::select(
                    TupleUnpacker::ensureTuple(std::get<0>(result)),
                    [](auto&& val) { return CodeGen::Statement { std::forward<decltype(val)>(val) }; },
                    providerIndex);

                return current;
            }*/
            throw 0;
        }

        value_argument_tuple mArguments;
        template <size_t I>
        const decayed_t<std::tuple_element_t<I, value_argument_tuple>> &getArguments() const
        {
            return std::get<I>(mArguments);
        }
        template <size_t I>
        void setArguments(decayed_t<std::tuple_element_t<I, value_argument_tuple>> v)
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