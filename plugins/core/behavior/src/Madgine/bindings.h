#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/fixed_string.h"

#include "behaviorerror.h"

#include "Meta/keyvalue/valuetype_desc.h"

namespace Engine {

struct get_binding_d_t {
    template <typename V, typename O>
    requires(!is_tag_invocable_v<get_binding_d_t, V &, std::string_view, O &>) auto operator()(V &v, std::string_view name, O &out) const
    {
        std::string errorMsg = "Binding \""s + std::string { name } + "\" not found.";
        return BehaviorError {
            BehaviorResult::UNKNOWN_ERROR,
            errorMsg };        
    }

    template <typename V, typename O>
    requires(is_tag_invocable_v<get_binding_d_t, V &, std::string_view, O &>) auto operator()(V &v, std::string_view name, O &out) const
        noexcept(is_nothrow_tag_invocable_v<get_binding_d_t, V &, std::string_view, O &>)
            -> tag_invoke_result_t<get_binding_d_t, V &, std::string_view, O &>
    {
        return tag_invoke(*this, v, name, out);
    }
};

inline constexpr get_binding_d_t get_binding_d;

template <fixed_string Name>
struct get_binding_t {

    template <typename V, typename O>
    requires(!is_tag_invocable_v<get_binding_t, V, O &>) decltype(auto) operator()(V &&v, O &out) const
    {
        return get_binding_d(std::forward<V>(v), Name, out);
    }

    template <typename V, typename O>
    requires(is_tag_invocable_v<get_binding_t, V, O &>) auto operator()(V &&v, O &out) const
        noexcept(is_nothrow_tag_invocable_v<get_binding_t, V, O &>)
            -> tag_invoke_result_t<get_binding_t, V, O &>
    {
        return tag_invoke(*this, std::forward<V>(v), out);
    }
};

template <fixed_string Name>
inline constexpr get_binding_t<Name> get_binding;

template <fixed_string Name, typename T>
struct Binding {
    template <typename Rec>
    struct state : Execution::base_state<Rec> {
        void start()
        {
            std::conditional_t<std::is_reference_v<T>, OutRef<std::remove_reference_t<T>>, T> result;

            BehaviorError error = get_binding<Name>(this->mRec, result);
            if (error.mResult == BehaviorResult { BehaviorResult::SUCCESS }){
                this->mRec.set_value(std::forward<T>(result));
            } else {
                this->mRec.set_error(std::move(error));
            }
        }
    };

    using is_sender = void;

    using result_type = BehaviorError;
    template <template <typename...> typename Tuple>
    using value_types = Tuple<T>;

    template <typename Rec>
    friend auto tag_invoke(Execution::connect_t, const Binding &sender, Rec &&rec)
    {
        return state<Rec> { std::forward<Rec>(rec) };
    }
};

template <fixed_string Name, typename T>
inline constexpr Binding<Name, T> binding;

struct BindingDescriptor {
    std::string mName;
    ExtendedValueTypeDesc mType;
};

template <fixed_string Name>
struct with_binding_t {

    template <typename Rec, typename F>
    struct receiver : Execution::algorithm_receiver<Rec> {

        template <typename O>
        friend auto tag_invoke(get_binding_d_t, receiver &rec, std::string_view name, O &out)
        {
            if (name == Name) {
                return rec.mBinding(out);
            } else {
                return get_binding_d(rec.mRec, name, out);
            }
        }

        F mBinding;
    };

    template <typename Inner, typename F>
    struct sender : Execution::algorithm_sender<Inner> {
        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
        {
            return Execution::algorithm_state<Inner, receiver<Rec, F>> { std::forward<Inner>(sender.mSender), std::forward<Rec>(rec), std::forward<F>(sender.mBinding) };
        }

        F mBinding;
    };

    
    template <typename Sender, typename F>
    friend auto tag_invoke(with_binding_t, Sender &&inner, F &&binding)
    {
        return sender<Sender, F> { { {}, std::forward<Sender>(inner) }, std::forward<F>(binding) };
    }

    template <typename Sender, typename F>
    requires tag_invocable<with_binding_t, Sender, F>
    auto operator()(Sender &&sender, F &&binding) const
        noexcept(is_nothrow_tag_invocable_v<with_binding_t, Sender, F>)
            -> tag_invoke_result_t<with_binding_t, Sender, F>
    {
        return tag_invoke(*this, std::forward<Sender>(sender), std::forward<F>(binding));
    }

    template <typename F>
    auto operator()(F &&binding) const
    {
        return pipable_from_right(*this, std::forward<F>(binding));
    }

};

template <fixed_string Name>
constexpr with_binding_t<Name> with_binding;

template <fixed_string Name>
constexpr auto with_constant_binding = [](auto &&value) {
    return with_binding<Name>([value { std::forward<decltype(value)>(value) }](auto &out) -> BehaviorError {
        out = value;
        return {};
    });
};

}