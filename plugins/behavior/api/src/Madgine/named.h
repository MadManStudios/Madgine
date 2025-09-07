#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/fixed_string.h"

#include "behaviorerror.h"

#include "Meta/keyvalue/valuetype_desc.h"
#include "Meta/keyvalue/valuetype_forward.h"

#include "Generic/callable_view.h"

#include "Generic/execution/storage.h"

namespace Engine {

template <typename T>
using NamedStorage = Execution::ResultStorageImpl<Execution::ValueStorageImpl<T>, Execution::ErrorStorageImpl<BehaviorError>>;

struct get_named_d_t {
    using signature = BehaviorError(std::string_view, ValueTypeRef);

    template <typename T>
        requires(!is_tag_invocable_v<get_named_d_t, T &, std::string_view, ValueTypeRef>)
    auto operator()(T &t, std::string_view name, ValueTypeRef out) const
    {
        std::string errorMsg = "Binding \""s + std::string { name } + "\" not found.";
        return BehaviorError {
            BehaviorResult::UNKNOWN_ERROR,
            errorMsg
        };
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

template <fixed_string Name, typename T>
struct get_named_t {

    template <typename O>
        requires(!is_tag_invocable_v<get_named_t, O>)
    decltype(auto) operator()(O &&o) const
    {
        NamedStorage<ValueType_Return<T>> storage;
        auto f = [&](ValueType &v) {
            BehaviorError result = get_named_d(std::forward<O>(o), Name, v);
            if (result.mResult == GenericResult::SUCCESS)
                storage.set_value(ValueType_as<T>(v));
            else
                storage.set_error(result);
        };
        ValueType_erased(CallableView<void(ValueType &)> { f });
        return storage;
    }

    template <typename O>
        requires(is_tag_invocable_v<get_named_t, O>)
    auto operator()(O &&o) const
        noexcept(is_nothrow_tag_invocable_v<get_named_t, O>)
            -> tag_invoke_result_t<get_named_t, O>
    {
        return tag_invoke(*this, std::forward<O>(o));
    }
};

template <fixed_string Name, typename T>
inline constexpr get_named_t<Name, T> get_named;

template <fixed_string Name, typename T>
struct Named {
    template <typename Rec>
    struct state : Execution::base_state<Rec> {
        void start()
        {
            get_named<Name, T>(this->mRec).reproduce(this->mRec);
        }

        void stop()
        {
            throw 0;
        }
    };
    
    using is_sender = void;

    using result_type = BehaviorError;
    template <template <typename...> typename Tuple>
    using value_types = Tuple<ValueType_Return<T>>;
    
    template <typename Rec>
    friend auto tag_invoke(Execution::connect_t, const Named &sender, Rec &&rec)
    {
        return state<Rec> { std::forward<Rec>(rec) };
    }
};

template <fixed_string Name, typename T>
inline constexpr Named<Name, T> named;

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
                return BehaviorError {};
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
            return Execution::algorithm_state<Inner, receiver<Rec, T>> { std::forward<Inner>(sender.mSender), std::forward<Rec>(rec), std::forward<T>(sender.mValue) };
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