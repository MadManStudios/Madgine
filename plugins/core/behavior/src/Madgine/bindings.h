#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/fixed_string.h"

#include "behaviorerror.h"

#include "Meta/keyvalue/valuetype_desc.h"
#include "Meta/keyvalue/valuetype_forward.h"

#include "Generic/callable_view.h"

namespace Engine {

struct get_binding_d_t {
    using signature = BehaviorError(std::string_view, ValueTypeRef);

    template <typename T>
    requires(!is_tag_invocable_v<get_binding_d_t, T&, std::string_view, ValueTypeRef>) auto operator()(T &t, std::string_view name, ValueTypeRef out) const
    {
        std::string errorMsg = "Binding \""s + std::string { name } + "\" not found.";
        return BehaviorError {
            BehaviorResult::UNKNOWN_ERROR,
            errorMsg };        
    }

    template <typename T>
    requires(is_tag_invocable_v<get_binding_d_t, T &, std::string_view, ValueTypeRef>) auto operator()(T &t, std::string_view name, ValueTypeRef out) const
        noexcept(is_nothrow_tag_invocable_v<get_binding_d_t, T &, std::string_view, ValueTypeRef>)
            -> tag_invoke_result_t<get_binding_d_t, T &, std::string_view, ValueTypeRef>
    {
        return tag_invoke(*this, t, name, out);
    }

    MADGINE_BEHAVIOR_EXPORT static BehaviorError type_erased(CallableView<BehaviorError(ValueType &)> cb);
};

inline constexpr get_binding_d_t get_binding_d;

template <fixed_string Name>
struct get_binding_t {

    template <typename T, typename O>
    requires(!is_tag_invocable_v<get_binding_t, T, O &>) decltype(auto) operator()(T &&t, O &out) const
    {
        if constexpr (std::same_as<O, ValueType> || std::same_as<O, ValueTypeRef>) {
            return get_binding_d(std::forward<T>(t), Name, out);
        } else {
            auto f = [&](ValueType &v) {
                BehaviorError result = get_binding_d(std::forward<T>(t), Name, v);
                if (result.mResult == GenericResult::SUCCESS)
                    out = ValueType_as<O>(v);
                return result;
            };
            return get_binding_d_t::type_erased(CallableView<BehaviorError(ValueType &)> { f });
        }
    }

    template <typename T, typename O>
    requires(is_tag_invocable_v<get_binding_t, T, O &>) auto operator()(T &&t, O &out) const
        noexcept(is_nothrow_tag_invocable_v<get_binding_t, T, O &>)
            -> tag_invoke_result_t<get_binding_t, T, O &>
    {
        return tag_invoke(*this, std::forward<T>(t), out);
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

        friend auto tag_invoke(get_binding_d_t, receiver &rec, std::string_view name, ValueTypeRef out)
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