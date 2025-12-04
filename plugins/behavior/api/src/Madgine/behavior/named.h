#pragma once

#include "Generic/callable_view.h"
#include "Generic/execution/concepts.h"
#include "Generic/execution/storage.h"
#include "Generic/fixed_string.h"

#include "Meta/keyvalue/valuetype_forward.h"

#include "behaviorerror.h"

namespace Engine {
namespace Behavior {

    struct get_named_d_t {
        using signature = bool(std::string_view, ValueTypeRef);

        template <typename T>
            requires(!is_tag_invocable_v<get_named_d_t, T &, std::string_view, ValueTypeRef>)
        auto operator()(T &t, std::string_view name, ValueTypeRef out) const
        {
            return false;
        }

        template <typename T>
            requires(is_tag_invocable_v<get_named_d_t, T &, std::string_view, ValueTypeRef>)
        auto operator()(T &t, std::string_view name, ValueTypeRef out) const
            noexcept(is_nothrow_tag_invocable_v<get_named_d_t, T &, std::string_view, ValueTypeRef>)
                -> tag_invoke_result_t<get_named_d_t, T &, std::string_view, ValueTypeRef>
        {
            return tag_invoke(*this, t, name, out);
        }
    };

    inline constexpr get_named_d_t get_named_d;

    template <fixed_string Name, typename V>
    struct get_named_t {

        template <typename T, typename O>
            requires(!is_tag_invocable_v<get_named_t, T, O>)
        bool operator()(T &&context, O &o) const
        {
            bool result;
            auto f = [&](ValueType &v) {
                result = get_named_d(std::forward<T>(context), Name, v);
                if (result)
                    o = ValueType_as<V>(v);
            };
            ValueType_erased(CallableView<void(ValueType &)> { f });
            return result;
        }

        template <typename T, typename O>
            requires(is_tag_invocable_v<get_named_t, T, O>)
        auto operator()(T &&context, O &&o) const
            noexcept(is_nothrow_tag_invocable_v<get_named_t, T, O>)
                -> tag_invoke_result_t<get_named_t, T, O>
        {
            return tag_invoke(*this, std::forward<T>(context), std::forward<O>(o));
        }
    };

    template <fixed_string Name, typename V>
    inline constexpr get_named_t<Name, V> get_named;

    template <typename Rec, fixed_string Name, typename T>
    struct NamedState : Execution::base_state<Rec> {

        NamedState(Named<Name, T> value, Rec &&rec)
            : Execution::base_state<Rec>(std::forward<Rec>(rec))
            , mValue(std::move(value))

        {
        }

        void start()
        {
            if (!mValue.mValue) {
                if (!mValue.resolve(this->mRec)) {
                    this->mRec.set_error(BEHAVIOR_UNKNOWN_ERROR() << "Named value '" << std::string(Name) << "' not found");
                    return;
                }
            }

            this->mRec.set_value(*mValue.mValue);
        }

        void stop()
        {
        }

        friend auto tag_invoke(Execution::visit_state_t, NamedState *state, auto &&visitor)
        {
        }

        Named<Name, T> mValue;
    };

    template <fixed_string Name, typename T>
    struct Named {

        using is_sender = void;

        using result_type = BehaviorError;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<ValueType_Return<T> &>;

        bool resolve(auto &context)
        {
            if (!mValue) {
                get_named<Name, T>(context, mValue);
            }
            return static_cast<bool>(mValue);
        }

        ValueType_Return<T> &operator->()
        {
            return *mValue;
        }

        ValueType_Return<T> &operator*()
        {
            return *mValue;
        }

        decltype(auto) operator->*(auto &&arg)
        {
            return *mValue->*std::forward<decltype(arg)>(arg);
        }

        operator ValueType_Return<T> &()
        {
            return *mValue;
        }

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, Named &&sender, Rec &&rec)
        {
            return NamedState<Rec, Name, T> { std::move(sender), std::forward<Rec>(rec) };
        }

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, Named &sender, Rec &&rec)
        {
            return NamedState<Rec, Name, T> { sender, std::forward<Rec>(rec) };
        }

        std::optional<forward_ref_t<ValueType_Return<T>>> mValue;
    };

    struct NamedDescriptor {
        std::string mName;
        ExtendedValueTypeDesc mType;
    };

    template <fixed_string Name>
    struct with_named_t {

        template <typename Rec, typename T>
        struct receiver : Execution::algorithm_receiver<Rec> {

            receiver(Rec &&rec, T &&value)
                : Execution::algorithm_receiver<Rec> { std::forward<Rec>(rec) }
                , mValue(std::forward<T>(value))
            {
            }

            friend auto tag_invoke(get_named_d_t, receiver &rec, std::string_view name, ValueTypeRef out)
            {
                if (name == Name) {
                    to_ValueType(out, rec.mValue);
                    return true;
                } else {
                    return get_named_d(rec.mRec, name, out);
                }
            }

            T mValue;
        };

        template <Execution::Sender Inner, typename T>
        struct sender : Execution::algorithm_sender<Inner> {
            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
            {
                return Execution::algorithm_state<Inner, receiver<Rec, T>> { std::forward<Inner>(sender.mSender), receiver<Rec, T> { std::forward<Rec>(rec), std::forward<T>(sender.mValue) } };
            }

            T mValue;
        };

        template <Execution::Sender Sender, typename T>
        friend auto tag_invoke(with_named_t, Sender &&inner, T &&value)
        {
            return sender<Sender, T> { { {}, std::forward<Sender>(inner) }, std::forward<T>(value) };
        }

        template <typename Sender, typename T>
            requires tag_invocable<with_named_t, Sender, T>
        auto operator()(Sender &&sender, T &&value) const
            noexcept(is_nothrow_tag_invocable_v<with_named_t, Sender, T>)
                -> tag_invoke_result_t<with_named_t, Sender, T>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<T>(value));
        }

        template <typename T>
        auto operator()(T &&value) const
        {
            return pipable_from_right(*this, std::forward<T>(value));
        }
    };

    template <fixed_string Name>
    constexpr with_named_t<Name> with_named;

}
}
