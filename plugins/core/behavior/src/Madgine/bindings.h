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

template <typename T>
decltype(auto) wrapBinding(T&& binding) {
    return [binding { std::forward<T>(binding) }](auto &out)->BehaviorError {
        out = binding;
        return {};
    };
}

template <fixed_string Name>
struct with_binding_t {

    template <typename Rec, typename T>
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

        T mBinding;
    };

    template <typename Inner, typename T>
    struct sender : Execution::algorithm_sender<Inner> {
        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
        {
            return Execution::algorithm_state<Inner, receiver<Rec, T>> { std::forward<Inner>(sender.mSender), std::forward<Rec>(rec), std::forward<T>(sender.mBinding) };
        }

        T mBinding;
    };

    
    template <typename Sender, typename T>
    friend auto tag_invoke(with_binding_t, Sender &&inner, T &&binding)
    {
        return sender<Sender, decltype(wrapBinding(std::forward<T>(binding)))> { { {}, std::forward<Sender>(inner) }, wrapBinding(std::forward<T>(binding)) };
    }

    template <typename Sender, typename T>
    requires tag_invocable<with_binding_t, Sender, T>
    auto operator()(Sender &&sender, T &&binding) const
        noexcept(is_nothrow_tag_invocable_v<with_binding_t, Sender, T>)
            -> tag_invoke_result_t<with_binding_t, Sender, T>
    {
        return tag_invoke(*this, std::forward<Sender>(sender), std::forward<T>(binding));
    }

    template <typename T>
    auto operator()(T &&binding) const
    {
        return pipable_from_right(*this, std::forward<T>(binding));
    }

};

template <fixed_string Name>
constexpr with_binding_t<Name> with_binding;

}