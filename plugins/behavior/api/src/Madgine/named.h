#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/fixed_string.h"

#include "Meta/keyvalue/valuetype_forward.h"

#include "Generic/callable_view.h"

#include "Generic/execution/storage.h"

namespace Engine {

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

template <typename Rec, fixed_string Name, typename T, typename F>
struct NamedState : Execution::base_state<Rec> {

    using Sender = std::invoke_result_t<F, ValueType_Return<T> &>;

    using State = Execution::connect_result_t<Sender, Rec>;

    NamedState(F &&f, Named<Name, T> value, Rec &&rec)
        : Execution::base_state<Rec>(std::forward<Rec>(rec))
        , mF(std::forward<F>(f))
        , mValue(std::move(value))

    {
        if (!mValue.mValue) {
            if (!mValue.resolve(this->mRec)) {
                throw 0;
            }
            construct(mState, DelayedConstruct<State> { [&]() { return Execution::connect(std::invoke(mF, *mValue), std::forward<Rec>(this->mRec)); } });
        }
    }

    ~NamedState()
    {
        if (mValue.mValue) {
            destruct(mState);
        }
    }

    void start()
    {
        if (!mValue.mValue) {
            if (!mValue.resolve(this->mRec)) {
                throw 0;
                //            mRec.set_error( { "Named value '" + std::string(Name) + "' not found" });
                return;
            }
            construct(mState, DelayedConstruct<State> { [&]() { return Execution::connect(std::invoke(mF, *mValue), std::forward<Rec>(this->mRec)); } });
        }

        mState->start();
    }

    void stop()
    {
        mState->stop();
    }

    template <typename... V>
    void set_value(V &&...value)
    {
        destruct(mState);
        this->mRec.set_value(std::forward<V>(value)...);
    }
    void set_done()
    {
        destruct(mState);
        this->mRec.set_done();
    }
    template <typename... R>
    void set_error(R &&...result)
    {
        destruct(mState);
        this->mRec.set_error(std::forward<R>(result)...);
    }

    friend auto tag_invoke(Execution::visit_state_t, NamedState *state, auto &&visitor)
    {
        visitor(Execution::State::Text { Name.c_str() });
        if (state) {
            Execution::visit_state(&state->mState, std::forward<decltype(visitor)>(visitor));
        } else {
            Execution::visit_state(static_cast<State *>(nullptr), std::forward<decltype(visitor)>(visitor));
        }
    }

    Named<Name, T> mValue;
    F mF;
    ManualLifetime<State> mState;
};

template <fixed_string Name, typename T, typename F>
struct NamedSender {

    using is_sender = void;

    using Inner = std::invoke_result_t<F, ValueType_Return<T> &>;

    using result_type = typename Inner::result_type;
    template <template <typename...> typename Tuple>
    using value_types = typename Inner::template value_types<Tuple>;

    template <typename Rec>
    friend auto tag_invoke(Execution::connect_t, NamedSender &&sender, Rec &&rec)
    {
        return NamedState<Rec, Name, T, F> { std::forward<F>(sender.mF), std::move(sender.mValue), std::forward<Rec>(rec) };
    }

    template <typename Rec>
    friend auto tag_invoke(Execution::connect_t, NamedSender &sender, Rec &&rec)
    {
        return NamedState<Rec, Name, T, F> { F { sender.mF }, sender.mValue, std::forward<Rec>(rec) };
    }

    Named<Name, T> mValue;
    F mF;
};

template <fixed_string Name, typename T>
struct Named {

    bool resolve(auto &context)
    {
        if (!mValue) {
            get_named<Name, T>(context, mValue);
        }
        return static_cast<bool>(mValue);
    }

    template <typename F>
    auto sender(F &&algorithm)
    {

        return NamedSender<Name, T, F> { *this, std::forward<F>(algorithm) };
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
