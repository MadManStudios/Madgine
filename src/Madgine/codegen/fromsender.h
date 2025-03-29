#pragma once

#include "Generic/execution/concepts.h"
//#include "Generic/execution/state.h"

namespace CodeGen {

struct CodeGen_Context {

    std::string generateName(std::string prefix)
    {
        return prefix + std::to_string(++mRunningIndex);
    }

    size_t mRunningIndex = 0;
};

using codegen_receiver = Engine::Execution::execution_receiver<CodeGen_Context>;

template <typename _Rec>
struct codegen_base_state {
    using Rec = _Rec;

    Rec mRec;

    template <typename CPO>
    friend decltype(auto) tag_invoke(CPO f, codegen_base_state &state)
    {
        return f(state.mRec);
    }
};

struct codegen_connect_t;

template <typename Sender, typename Rec>
using codegen_connect_result_t = tag_invoke_result_t<codegen_connect_t, Sender, Rec>;

template <typename State>
struct codegen_algorithm_state_helper {

    using InnerRec = typename State::Rec;
    using Rec = InnerRec;

    template <typename Sender, typename... Args>
    codegen_algorithm_state_helper(Sender &&sender, Rec &&rec, Args &&...args)
        : mState { codegen_connect(std::forward<Sender>(sender), InnerRec { std::forward<Rec>(rec), std::forward<Args>(args)... }) }
    {
    }

    ~codegen_algorithm_state_helper() { }

    State mState;

    template <typename CPO>
    friend decltype(auto) tag_invoke(CPO f, codegen_algorithm_state_helper &state)
    {
        return f(state.mState);
    }
};

template <typename Sender, typename InnerRec>
using codegen_algorithm_state = codegen_algorithm_state_helper<codegen_connect_result_t<Sender, InnerRec>>;

extern const codegen_connect_t codegen_connect;

struct codegen_connect_t {

    /* template <Engine::fixed_string Name, typename Sender, typename T, typename Rec>
    friend auto tag_invoke(codegen_connect_t, Engine::Execution::Variable_t::sender<Name, Sender, T> &&sender, Rec &&rec)
    {
        struct state : codegen_algorithm_state<Sender, Rec> {
            auto generate()
            {
                return std::tuple_cat(
                    std::make_tuple(VariableDefinition { Variable { std::string { Name }, Engine::toValueTypeDesc<Engine::decayed_t<T>>() }, Constant { Engine::ValueType { mInitialValue } } }),
                    this->mState.generate());
            }
            Engine::decayed_t<T> mInitialValue;
        };
        return state { { std::move(sender.mSender), std::forward<Rec>(rec) }, std::forward<Engine::decayed_t<T>>(sender.mInitialValue) };
    }

    template <Engine::fixed_string Name, typename T, typename Rec>
    friend auto tag_invoke(codegen_connect_t, Engine::Execution::read_var_t::sender<Name, T> &&sender, Rec &&rec)
    {
        struct state : codegen_base_state<Rec> {
            auto generate()
            {
                return std::make_tuple(VariableAccess { std::string { Name } });
            }
        };
        return state { std::forward<Rec>(rec) };
    }

    template <Engine::fixed_string Name, typename Sender, typename Rec>
    friend auto tag_invoke(codegen_connect_t, Engine::Execution::write_var_t::sender<Name, Sender> &&sender, Rec &&rec)
    {
        struct state : codegen_algorithm_state<Sender, Rec> {
            auto generate()
            {
                auto inner = this->mState.generate();
                static_assert(std::tuple_size_v<decltype(inner)> == 1);
                return std::make_tuple(Assignment { {}, VariableAccess { std::string { Name } }, std::get<0>(inner) });
            }
        };
        return state { { std::move(sender.mSender), std::forward<Rec>(rec) } };
    }*/

    struct codegen_let_value_t {
        template <typename T>
        static Engine::ExtendedValueTypeDesc typeHelper()
        {
            if constexpr (Engine::ValueTypePrimitive<std::remove_reference_t<T>>)
                return Engine::toValueTypeDesc<std::remove_reference_t<T>>();
            else
                return { Engine::ExtendedValueTypeEnum::GenericType };
        }

        template <typename Sender, typename F, typename Rec>
        struct state : codegen_base_state<Rec> {
            template <typename... V, size_t... Is>
            auto generateHelper(Engine::type_pack<V...>, std::index_sequence<Is...>)
            {
                std::string variableName = Engine::Execution::get_context(*this).generateName("v");
                auto inner = codegen_connect(std::move(mSender), this->mRec).generate();
                static_assert(std::tuple_size_v<decltype(inner)> == sizeof...(V));
                return std::tuple_cat(
                    std::make_tuple(VariableDefinition { { variableName + "_" + std::to_string(Is), typeHelper<V>(), std::is_reference_v<V> }, std::get<Is>(inner) }...),
                    codegen_connect(mF((variableName + "_" + std::to_string(Is))...), this->mRec).generate());
            }

            auto generate()
            {
                using Pack = typename Sender::template value_types<Engine::type_pack>;
                return generateHelper(Pack {}, typename Pack::indices {});
            }

            Sender mSender;
            F mF;
        };
    };

    template <typename Sender, typename F, typename Rec>
    friend auto tag_invoke(codegen_connect_t, Engine::Execution::let_value_t::sender<Sender, F> &&sender, Rec &&rec)
    {
        return codegen_let_value_t::state<Sender, F, Rec> { std::forward<Rec>(rec), std::move(sender.mSender), std::forward<F>(sender.mF) };
    }

    template <typename Sender, typename T, typename Rec>
    friend auto tag_invoke(codegen_connect_t, Engine::Execution::assign_t::sender<Sender, T> &&sender, Rec &&rec)
    {
        struct state : codegen_algorithm_state<Sender, Rec> {
            auto generate()
            {
                auto inner = this->mState.generate();
                static_assert(std::tuple_size_v<decltype(inner)> == 1);
                return std::make_tuple(Assignment { {}, VariableAccess { "TODO" }, std::get<0>(inner) });
            }
            T mVar;
        };
        return state { { std::move(sender.mSender), std::forward<Rec>(rec) }, std::forward<T>(sender.mVar) };
    }

    template <typename... Sender, typename Rec>
    friend auto tag_invoke(codegen_connect_t, Engine::Execution::sequence_t::sender<Sender...> &&sender, Rec &&rec)
    {
        struct state : codegen_base_state<Rec> {

            auto generate()
            {
                return std::tuple_cat(
                    codegen_connect(std::get<Sender>(std::move(mSenders)), this->mRec).generate()...);
            }
            std::tuple<Sender...> mSenders;
        };
        return state { std::forward<Rec>(rec), std::move(sender.mSenders) };
    }

    template <typename... Args, typename Rec>
    friend auto tag_invoke(codegen_connect_t, Engine::Execution::just_t::sender<Args...> &&sender, Rec &&rec)
    {
        struct state : codegen_base_state<Rec> {
            auto generate()
            {
                return [this]<size_t... Is>(std::index_sequence<Is...>)
                {
                    return std::make_tuple(CodeGen::Constant { std::get<Is>(mArgs)... });
                }
                (std::index_sequence_for<Args...> {});
            }
            std::tuple<Args...> mArgs;
        };
        return state { std::forward<Rec>(rec), std::move(sender.mArgs) };
    }

    template <typename Sender, typename T, typename Rec>
    friend auto tag_invoke(codegen_connect_t, Engine::Execution::then_t::sender<Sender, T> &&sender, Rec &&rec)
    {
        struct state : codegen_algorithm_state<Sender, Rec> {
            auto generate()
            {
                return std::make_tuple(
                    Engine::TupleUnpacker::invokeExpand(mTransform, this->mState.generate()));
            }
            T mTransform;
        };
        return state { { std::move(sender.mSender), std::forward<Rec>(rec) }, std::forward<T>(sender.mTransform) };
    }

    template <typename Stream, typename T, typename F, typename Rec>
    friend auto tag_invoke(codegen_connect_t, Engine::Execution::reduce_stream_t::sender<Stream, T, F> &&sender, Rec &&rec)
    {
        struct state {
            auto generate()
            {
                return std::make_tuple();
            }
           
        };
        return state { };
    }

    template <typename C, typename F, typename Rec>
    friend auto tag_invoke(codegen_connect_t, Engine::Execution::for_each_t::sender<C, F> &&sender, Rec &&rec)
    {
        struct state : codegen_base_state<Rec> {
            auto generate()
            {
                std::string variableName = Engine::Execution::get_context(*this).generateName("e");
                VariableAccess read { variableName };
                ForEach block { {}, VariableAccess { "TODO" }, variableName };
                Engine::TupleUnpacker::forEach(codegen_connect(mF(variableName), std::forward<Rec>(this->mRec)).generate(), [&](auto s) {
                    block.mBody.push_back(s);
                });
                return std::make_tuple(block);
            }
            C mContainer;
            F mF;
        };
        return state { std::forward<Rec>(rec), std::forward<C>(sender.mContainer), std::forward<F>(sender.mF) };
    }

    template <typename Sender, typename Rec>
    requires tag_invocable<codegen_connect_t, Sender, Rec>
    auto operator()(Sender &&sender, Rec &&rec) const
        noexcept(is_nothrow_tag_invocable_v<codegen_connect_t, Sender, Rec>)
            -> tag_invoke_result_t<codegen_connect_t, Sender, Rec>
    {
        return tag_invoke(*this, std::forward<Sender>(sender), std::forward<Rec>(rec));
    }
};

inline constexpr codegen_connect_t codegen_connect;

struct codegen_for_each_t {

    template <typename C, typename F>
    struct sender {
        //using value_type = typename std::remove_reference_t<C>::value_type &;
        //using inner_sender_t = std::invoke_result_t<F, value_type>;
        using result_type = void;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(codegen_connect_t, sender &&sender, Rec &&rec)
        {
            struct state : codegen_base_state<Rec> {
                auto generate()
                {
                    std::string variableName = Engine::Execution::get_context(*this).generateName("e");
                    VariableAccess read { variableName };
                    ForEach block { {}, mContainer, variableName };
                    Engine::TupleUnpacker::forEach(codegen_connect(mF(read), std::forward<Rec>(this->mRec)).generate(), [&](auto s) {
                        block.mBody.push_back(s);
                    });
                    return std::make_tuple(block);
                }
                C mContainer;
                F mF;
            };
            return state { std::forward<Rec>(rec), std::forward<C>(sender.mContainer), std::forward<F>(sender.mF) };
        }
        C mContainer;
        F mF;
    };
};

template <typename Sender, typename Rec = codegen_receiver>
auto generatorFromSender(Sender &&sender, Rec rec = {})
{
    return codegen_connect(std::move(sender), rec);
}

}