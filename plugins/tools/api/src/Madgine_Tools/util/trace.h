#pragma once

#include "Meta/reflect/value.h"

#include "keyvaluevalueoperation.h"

namespace Engine {
namespace Tools {

    struct Trace {

        std::string name()
        {
            return "TODO";
        }

        Reflect::Result follow();

        Closure<Reflect::Result()> mTrace;
    };

    template <typename Parent, typename U>
    struct TracedVariantAccess;

    template <typename T, typename F, typename... Args>
    struct TracedAccess : Traced<std::invoke_result_t<F, std::remove_reference_t<T> &, Args...>> {
        using V = std::invoke_result_t<F, std::remove_reference_t<T> &, Args...>;

        using Callback = bool (*)(const TracedAccess<T, F, Args...> &, bool);

        TracedAccess(const Traced<T> &parent, F f, Callback callback = nullptr, std::tuple<Args...> args = {})
            : Traced<V>(TupleUnpacker::invokeExpand(std::forward<F>(f), parent.get(), args))
            , mParent(parent)
            , mF(std::forward<F>(f))
            , mCallback(callback)
            , mArgs(std::move(args))
        {
        }

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<V> &)>)> vBuild() const override
        {
            return build();
        }

        auto build() const
        {
            return [parent { mParent.build() }, f { mF }, args { mArgs }, callback { mCallback }](CallableView<std::pair<Reflect::Result, bool>(const Traced<V> &)> tracer) {
                return parent([&](const Traced<T> &parent) {
                    TracedAccess access { parent, f, callback, args };
                    auto [result, changed] = tracer(access);
                    if (callback)
                        changed = callback(access, changed);
                    return std::make_pair(result, changed);
                });
            };
        }

        std::ostream &print(std::ostream &out) const override
        {
            return out << mParent << "\nAccess(" << typeid(F).name() << ")";
        }

        UndoStack &undoStack() const override
        {
            return mParent.undoStack();
        }

        const Traced<T> &mParent;
        F mF;
        Callback mCallback;
        std::tuple<Args...> mArgs;
    };

    template <typename T, typename V>
    struct TracedCast : Traced<V> {
        TracedCast(const Traced<T> &parent)
            : Traced<V>(parent.get())
            , mParent(parent)
        {
        }

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<V> &)>)> vBuild() const override
        {
            return build();
        }

        auto build() const
        {
            return [parent { mParent.build() }](CallableView<std::pair<Reflect::Result, bool>(const Traced<V> &)> tracer) { return parent([&](const Traced<T> &parent) { return tracer(TracedCast { parent }); }); };
        }

        std::ostream &print(std::ostream &stream) const override
        {
            return stream << mParent;
        }

        UndoStack &undoStack() const override
        {
            return mParent.undoStack();
        }

        const Traced<T> &mParent;
    };

    template <typename T, typename It>
    struct TracedSentinel : Traced<It> {
        TracedSentinel(const Traced<T> &parent)
            : Traced<It>(parent->end())
            , mParent(parent)
        {
        }

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<It> &)>)> vBuild() const override
        {
            return build();
        }

        auto build() const
        {
            return [parent { mParent.build() }](CallableView<std::pair<Reflect::Result, bool>(const Traced<It> &)> tracer) { return parent([&](const Traced<T> &parent) { return tracer(TracedSentinel { parent }); }); };
        }

        std::ostream &print(std::ostream &out) const override
        {
            return out << mParent << "\nend()";
        }

        UndoStack &undoStack() const override
        {
            return mParent.undoStack();
        }

        const Traced<T> &mParent;
    };

    template <typename T, typename It>
    struct TracedIterator : Traced<It> {
        TracedIterator(const Traced<T> &parent, It it)
            : Traced<It>(std::forward<It>(it))
            , mParent(parent)
        {
        }

        TracedIterator &operator++()
        {
            ++this->mValue;
            return *this;
        }

        TracedIterator &operator=(const TracedIterator &other)
        {
            assert(std::addressof(mParent) == std::addressof(other.mParent));
            this->mValue = other.mValue;
            return *this;
        }

        template <typename S>
        bool operator!=(const TracedIterator<T, S> &other) const
        {
            return this->mValue != other.mValue;
        }

        template <typename S>
        bool operator!=(const TracedSentinel<T, S> &other) const
        {
            return this->mValue != other.get();
        }

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<It> &)>)> vBuild() const override
        {
            return build();
        }

        auto build() const
        {
            return [parent { mParent.build() }, i { std::ranges::distance(mParent->begin(), this->mValue) }](CallableView<std::pair<Reflect::Result, bool>(const Traced<It> &)> tracer) { return parent([&](const Traced<T> &parent) { return tracer(TracedIterator { parent, std::next(parent->begin(), i) }); }); };
        }

        std::ostream &print(std::ostream &out) const override
        {
            return out << mParent << "\n[" << std::ranges::distance(mParent->begin(), this->mValue) << "]";
        }

        UndoStack &undoStack() const override
        {
            return mParent.undoStack();
        }

        const Traced<T> &mParent;
    };

    template <typename T, typename V>
    struct TracedBinding : Traced<V> {

        TracedBinding(const Traced<T> &parent, V &&v)
            : Traced<V>(std::forward<V>(v))
            , mParent(parent)
        {
        }

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<V> &)>)> vBuild() const override
        {
            return build();
        }

        auto build() const
        {
            return [parent { mParent.build() }](CallableView<std::pair<Reflect::Result, bool>(const Traced<V> &)> tracer) {
                return parent([&](const Traced<T> &parent) {
                    Reflect::Result result;
                    if (!Execution::access_binding(parent.get(), [&](auto &&v) {
                            result = tracer(TracedBinding { parent, std::forward<decltype(v)>(v) }).first;
                        })) {
                        result = REFLECT_UNKNOWN_ERROR();
                    }
                    return std::make_pair(result, false);
                });
            };
        }

        std::ostream &print(std::ostream &out) const override
        {
            return out << mParent << "(bound)";
        }

        UndoStack &undoStack() const override
        {
            return mParent.undoStack();
        }

        const Traced<T> &mParent;
    };

    template <typename V, size_t I>
    constexpr auto get_helper = []() {
        if constexpr (std::is_reference_v<V>)
            return [](V v) -> std::tuple_element_t<I, std::remove_reference_t<V>> & { return std::get<I>(static_cast<V>(v)); };
        else
            return [](V v) -> std::tuple_element_t<I, V> { return std::get<I>(static_cast<V>(v)); };
    }();

    template <typename V, size_t I>
    constexpr auto member_get_helper = []() {
        if constexpr (std::is_reference_v<V>)
            return [](V v) -> std::tuple_element_t<I, std::remove_reference_t<V>> & { return v.template get<I>(); };
        else
            return [](V v) -> std::tuple_element_t<I, V> { return static_cast<V>(v).template get<I>(); };
    }();

    template <typename T>
    struct TracedStorage {
    protected:
        TracedStorage() = default;
        TracedStorage(T value)
            : mValue(std::forward<T>(value))
        {
        }

        mutable T mValue;
    };

    template <typename T>
    struct TracedStorage<T &> {
    protected:
        TracedStorage() = default;
        TracedStorage(T &value)
            : mValue(value)
        {
        }

        T &mValue;
    };

    template <typename T>
    struct Traced : TracedStorage<T> {

        using meta_t = std::decay_t<T>;

        using traced_type = T;

        using TracedStorage<T>::TracedStorage;

        template <typename F, typename... Args>
        TracedAccess<T, F, Args...> trace(F &&f, Args... args) const
        {
            return { *this, std::forward<F>(f), nullptr, { args... } };
        }

        template <typename F>
        TracedAccess<T, F> traceEx(F &&f, bool (*tracer)(const TracedAccess<T, F> &, bool)) const
        {
            return { *this, std::forward<F>(f), tracer };
        }

        template <typename R, std::same_as<std::decay_t<T>> _T>
            requires std::is_const_v<std::remove_reference_t<T>>
        TracedAccess<T, R (_T::*)() const> trace(R (_T::*f)() const) const
        {
            return { *this, f };
        }

        template <typename R, std::same_as<std::decay_t<T>> _T>
            requires(!std::is_const_v<std::remove_reference_t<T>>)
        TracedAccess<T, R (_T::*)()> trace(R (_T::*f)()) const
        {
            return { *this, f };
        }

        template <typename R, std::convertible_to<T> _T>
        TracedAccess<T, R (*)(_T)> trace(R (*f)(_T)) const
        {
            return { *this, f };
        }

        void track(std::decay_t<T> value) const
        {
            undoStack().addOperation(std::make_unique<ReflectValueOperation<std::decay_t<T>>>(static_cast<const Traced<std::decay_t<T> &> &>(*this).build(), std::forward<T>(value)));
        }

        void trackContinuous(std::decay_t<T> value) const
        {
            UndoableOperation *op = undoStack().getContinuousOperation();
            if (op) {
                assert(dynamic_cast<ReflectValueOperation<std::decay_t<T>> *>(op));
            } else {
                undoStack().addContinuousOperation(std::make_unique<ReflectValueOperation<std::decay_t<T>>>(static_cast<const Traced<std::decay_t<T> &> &>(*this).build(), std::forward<T>(value)));
            }
        }

        void submitContinuous() const
        {
            undoStack().commitContinuousOperation();
        }

        virtual UndoStack &undoStack() const = 0;

        virtual Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<T> &)>)> vBuild() const = 0;

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<T> &)>)> build() const
        {
            return vBuild();
        }

        T &get() const
        {
            return this->mValue;
        }

        decltype(auto) operator->() const
        {
            return &this->mValue;
        }

        auto operator*() const
        {
            return trace([](std::remove_reference_t<T> &t) -> decltype(auto) { return *t; });
        }

        decltype(auto) operator[](auto v) const
        {
            return trace([](std::remove_reference_t<T> &t, auto v) -> decltype(auto) { return t[v]; }, v);
        }

        auto operator&() const
        {
            return trace([](std::remove_reference_t<T> &t) -> decltype(auto) { return &t; });
        }

        auto begin() const
            requires requires(T t) { t.begin(); }
        {
            return TracedIterator<T, decltype(std::declval<T>().begin())> { *this, this->mValue.begin() };
        }

        auto end() const
            requires requires(T t) { t.end(); }
        {
            return TracedSentinel<T, decltype(std::declval<T>().end())> { *this };
        }

        auto erase(const auto &it) const
            requires requires(T t) { t.begin(); }
        {
            throw 0;
            return TracedIterator<T, decltype(std::declval<T>().begin())> { *this, this->mValue.erase(it.get()) };
        }

        operator TracedCast<T, T &>() const
            requires(!std::is_const_v<std::remove_reference_t<T>>)
        {
            return { *this };
        }

        operator TracedCast<T, const std::remove_reference_t<T> &>() const
        {
            return { *this };
        }

        virtual std::ostream &print(std::ostream &stream) const = 0;

        friend std::ostream &operator<<(std::ostream &stream, const Traced<T> &trace)
        {
            return trace.print(stream);
        }

        friend bool tag_invoke(Execution::access_binding_t access, const Traced<T> &trace, auto &&visitor)
            requires tag_invocable<Execution::access_binding_t, const T &, decltype(visitor)>
        {
            return access(trace.mValue, [&](auto &&v) {
                return visitor(TracedBinding<T, decltype(v)> { trace, std::forward<decltype(v)>(v) });
            });
        }

        template <size_t I>
        auto get() const
        {
            if constexpr (requires(T &&v) { v.template get<I>(); }) {
                return trace(member_get_helper<T, I>);
            } else {
                return trace(get_helper<T, I>);
            }
        }
    };

    template <>
    struct Traced<Reflect::Value &> {

        using meta_t = Reflect::Value &;

        using traced_type = Reflect::Value &;

        Traced(Reflect::Value &value)
            : mValue(value)
        {
        }

        template <typename F>
        TracedAccess<Reflect::Value &, F> trace(F &&f) const
        {
            return { *this, std::forward<F>(f) };
        }

        virtual UndoStack &undoStack() const = 0;

        virtual Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<Reflect::Value &> &)>)> vBuild() const = 0;

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<Reflect::Value &> &)>)> build() const
        {
            return vBuild();
        }

        Reflect::Value &get() const
        {
            return mValue;
        }

        Reflect::Value *operator->() const
        {
            return &mValue;
        }

        virtual std::ostream &print(std::ostream &stream) const = 0;

        friend std::ostream &operator<<(std::ostream &stream, const Traced<Reflect::Value &> &trace)
        {
            return trace.print(stream);
        }

        template <typename V>
        decltype(auto) visit(V &&visitor) const
        {
            return mValue.visit([&](auto &&v) {
                return visitor(TracedVariantAccess<const Traced<Reflect::Value &> &, decltype(v)> { *this, std::forward<decltype(v)>(v) });
            });
        }

        Reflect::Value &mValue;
    };

    template <typename T>
    struct TracedRoot : Traced<T> {
        TracedRoot(UndoStack &stack, T value = {})
            : Traced<T>(std::forward<T>(value))
            , mUndoStack(stack)
        {
        }

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<T> &)>)> vBuild() const override
        {
            return build();
        }

        auto build() const
        {
            return [value { this->mValue }, &stack { mUndoStack }](CallableView<std::pair<Reflect::Result, bool>(const Traced<T> &)> tracer) { return tracer(TracedRoot { stack, value }).first; };
        }

        std::ostream &print(std::ostream &stream) const override
        {
            if constexpr (requires { stream << this->mValue; }) {
                return stream << "Root: " << this->mValue;
            } else {
                return stream << "Root: " << typeid(T).name();
            }
        }

        UndoStack &undoStack() const override
        {
            return mUndoStack;
        }

        UndoStack &mUndoStack;
    };

    template <typename T>
    struct TracedValueTypeCall : Traced<const T &> {

        TracedValueTypeCall(const Traced<const Reflect::Value&> &parent, const T &v)
            : Traced<const T &>(v)
            , mParent(parent)
        {
        }

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<const T &> &)>)> vBuild() const override
        {
            return build();
        }

        auto build() const
        {
            return [parent { mParent.build() }](CallableView<std::pair<Reflect::Result, bool>(const Traced<const T &> &)> tracer) {
                return parent([&](const Traced<const Reflect::Value &> &parent) {
                    bool changed = false;
                    Reflect::Result result = Reflect::call([&](const T &v) {
                        Reflect::Result result;
                        std::tie(result, changed) = tracer(Tools::TracedValueTypeCall { parent, v });
                        return result;
                    },
                        parent.get());
                    return std::make_pair(result, changed);
                });
            };
        }

        std::ostream &print(std::ostream &out) const override
        {
            return out << mParent << " -> ValueType_call";
        }

        UndoStack &undoStack() const override
        {
            return mParent.undoStack();
        }

        const Traced<const Reflect::Value&> &mParent;
    };

    template <typename T>
    struct TracedToValueType : Traced<Reflect::Value> {

        TracedToValueType(const Traced<T> &parent, Reflect::Value v)
            : Traced<Reflect::Value>(std::move(v))
            , mParent(parent)
        {
        }

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<Reflect::Value> &)>)> vBuild() const override
        {
            return build();
        }

        auto build() const
        {
            return [parent { mParent.build() }](CallableView<std::pair<Reflect::Result, bool>(const Traced<Reflect::Value> &)> tracer) {
                return parent([&](const Traced<const T &> &parent) {
                    Reflect::Value v;
                    toValue(v, parent.get());
                    return tracer(Tools::TracedToValueType { parent, std::move(v) });
                });
            };
        }

        std::ostream &print(std::ostream &out) const override
        {
            return out << mParent << " -> ValueType_call";
        }

        UndoStack &undoStack() const override
        {
            return mParent.undoStack();
        }

        const Traced<const T &> &mParent;
    };

    template <typename Parent, typename T>
    struct TracedVariantAccess : Traced<T> {

        TracedVariantAccess(Parent parent, T t)
            : Traced<T>(std::forward<T>(t))
            , mParent(std::forward<Parent>(parent))
        {
        }

        Closure<Reflect::Result(CallableView<std::pair<Reflect::Result, bool>(const Traced<T> &)>)> vBuild() const override
        {
            return build();
        }

        auto build() const
        {
            return [parent { mParent.build() }](CallableView<std::pair<Reflect::Result, bool>(const Traced<T> &)> tracer) { return parent([&](Parent parent) { return tracer(TracedVariantAccess { parent, Value_as<std::decay_t<T>>(parent.get()) }); }); };
        }

        std::ostream &print(std::ostream &stream) const override
        {
            return stream << mParent << " -> " << typeid(std::decay_t<T>).name();
        }

        UndoStack &undoStack() const override
        {
            return mParent.undoStack();
        }

        Parent mParent;
    };

}

namespace Reflect {
    template <typename Callable, typename Arg>
    Result call(Callable &&callable, const Tools::Traced<Arg> &arg)
    {
        using T = meta_decayed_t<std::decay_t<typename CallableTraits<Callable>::argument_types::template unpack_unique<>>>;

        return call([&](const T &v) { return callable(Tools::TracedValueTypeCall<T> { arg, v }); }, arg.get());
    }

    template <typename T>
    auto Value_as(const Tools::Traced<const Value &> &v)
    {
        return Tools::TracedVariantAccess<const Tools::Traced<const Value &> &, const T &> { v, Value_as<T>(v.get()) };
    }

    template <typename T>
    auto toValue(const Tools::Traced<T> &t)
    {
        Value v;
        toValue(v, t.get());
        return Tools::TracedToValueType<T> { t, std::move(v) };
    }
}

}

namespace std {
template <typename T>
    requires requires { typename T::traced_type; }
struct tuple_size<T> : tuple_size<std::remove_reference_t<typename T::traced_type>> { };

template <size_t I, typename T>
    requires(requires { typename T::traced_type; } && requires(T::traced_type t) { t.template get<I>(); })
struct tuple_element<I, T> {
    using type = Engine::Tools::TracedAccess<typename T::traced_type, decltype(Engine::Tools::member_get_helper<typename T::traced_type, I>) &>;
};

template <size_t I, typename T>
    requires(requires { typename T::traced_type; } && !requires(T::traced_type t) { t.template get<I>(); })
struct tuple_element<I, T> {
    using type = Engine::Tools::TracedAccess<typename T::traced_type, decltype(Engine::Tools::get_helper<typename T::traced_type, I>) &>;
};

}