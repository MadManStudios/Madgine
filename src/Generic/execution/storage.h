#pragma once

namespace Engine {
namespace Execution {

    template <typename... V>
    struct ValueStorageImpl {
        template <typename... U>
        ValueStorageImpl(U &&...u)
            : mValues(std::forward<U>(u)...)
        {
        }

        auto reproduce(auto &rec) &&
        {
            return TupleUnpacker::invokeExpand(LIFT(rec.set_value, &), std::move(mValues));
        }

        auto reproduce(auto &rec) &
        {
            return TupleUnpacker::invokeExpand(LIFT(rec.set_value, &), mValues);
        }

        template <size_t I>
        decltype(auto) get() &&
        {
            return std::get<I>(mValues);
        }

        const std::tuple<V...> &get() const &
        {
            return mValues;
        }

        std::tuple<V...> mValues;
    };

    template <typename V>
    struct ValueStorageImpl<V> {
        template <typename... U>
        ValueStorageImpl(U &&...u)
            : mValues(std::forward<U>(u)...)
        {
        }

        auto reproduce(auto &rec) &&
        {
            return rec.set_value(std::move(std::get<0>(mValues)));
        }

        auto reproduce(auto &rec) &
        {
            return rec.set_value(std::get<0>(mValues));
        }

        operator V() &&
        {
            return std::forward<V>(std::get<0>(mValues));
        }

        V get() &&
        {
            return std::forward<V>(std::get<0>(mValues));
        }

        operator const V &() const &
        {
            return std::get<0>(mValues);
        }

        const V &get() const &
        {
            return std::get<0>(mValues);
        }

        std::tuple<V> mValues;
    };

    template <typename R>
    struct ErrorStorageImpl {

        static constexpr bool is_void = false;

        template <typename V>
        ErrorStorageImpl(V &&e)
            : mError(std::forward<V>(e))
        {
        }

        auto reproduce(auto &rec) &&
        {
            return rec.set_error(std::move(mError));
        }

        auto reproduce(auto &rec) &
        {
            return rec.set_error(mError);
        }

        R mError;
    };

    template <>
    struct ErrorStorageImpl<void> {
        ErrorStorageImpl() = delete;

        static constexpr bool is_void = true;

        auto reproduce(auto &rec) -> decltype(rec.set_done())
        {
            throw 0;
        }
    };

    template <typename Sender>
    using ValueStorage = typename std::decay_t<Sender>::template value_types<ValueStorageImpl>;

    template <typename Sender>
    using ErrorStorage = ErrorStorageImpl<typename std::decay_t<Sender>::result_type>;

    struct DoneStorage {
        auto reproduce(auto &rec)
        {
            return rec.set_done();
        }
    };

    struct NullStorage {
        auto reproduce(auto &rec) -> decltype(rec.set_done())
        {
            throw 0;
        }
    };

    template <typename Value, typename Error>
    struct ResultStorageImpl {

        static constexpr bool can_have_error = !Error::is_void;

        auto reproduce(auto &rec) &&
        {
            return std::visit([&](auto &&storage) {
                using R = decltype(std::move(storage).reproduce(rec));
                if constexpr (std::same_as<R, std::bool_constant<true>> || std::same_as<R, std::bool_constant<false>>) {
                    return bool { std::move(storage).reproduce(rec) };
                } else {
                    return std::move(storage).reproduce(rec);
                }
            },
                std::move(mState));
        }

        auto reproduce(auto &rec) &
        {
            return std::visit([&](auto &storage) {
                using R = decltype(storage.reproduce(rec));
                if constexpr (std::same_as<R, std::bool_constant<true>> || std::same_as<R, std::bool_constant<false>>) {
                    return bool { storage.reproduce(rec) };
                } else {
                    return storage.reproduce(rec);
                }
            },
                mState);
        }

        bool is_null() const
        {
            return std::holds_alternative<NullStorage>(mState);
        }

        bool is_value() const
        {
            return std::holds_alternative<Value>(mState);
        }

        bool is_error() const
        {
            return std::holds_alternative<Error>(mState);
        }

        bool is_done() const
        {
            return std::holds_alternative<DoneStorage>(mState);
        }

        void reset()
        {
            mState.template emplace<NullStorage>();
        }

        template <typename... V>
        void set_value(V &&...v)
        {
            assert(std::holds_alternative<NullStorage>(mState));
            mState.template emplace<Value>(std::forward<V>(v)...);
        }

        template <typename... R>
        void set_error(R &&...r)
        {
            assert(std::holds_alternative<NullStorage>(mState));
            mState.template emplace<Error>(std::forward<R>(r)...);
        }

        void set_done()
        {
            assert(std::holds_alternative<NullStorage>(mState));
            mState.template emplace<DoneStorage>();
        }

        Value value() &&
        {
            return std::move(std::get<Value>(mState));
        }

        Error error() &&
        {
            return std::move(std::get<Error>(mState));
        }

        DoneStorage done() &&
        {
            return std::move(std::get<DoneStorage>(mState));
        }

        Value &value() &
        {
            return std::get<Value>(mState);
        }

        Error &error() &
        {
            return std::get<Error>(mState);
        }

        DoneStorage &done() &
        {
            return std::get<DoneStorage>(mState);
        }

        const Value &value() const &
        {
            return std::get<Value>(mState);
        }

        const Error &error() const &
        {
            return std::get<Error>(mState);
        }

        const DoneStorage &done() const &
        {
            return std::get<DoneStorage>(mState);
        }

        std::variant<NullStorage, Value, Error, DoneStorage> mState;
    };

    template <typename Sender>
    using ResultStorage = ResultStorageImpl<ValueStorage<Sender>, ErrorStorage<Sender>>;

}
}

namespace std {
template <typename V1, typename V2, typename... V>
struct tuple_size<Engine::Execution::ValueStorageImpl<V1, V2, V...>> : std::integral_constant<size_t, sizeof...(V) + 2> { };

template <typename V1, typename... V>
struct tuple_element<0, Engine::Execution::ValueStorageImpl<V1, V...>> {
    using type = V1;
};
template <size_t I, typename V1, typename... V>
struct tuple_element<I, Engine::Execution::ValueStorageImpl<V1, V...>> : std::tuple_element<I - 1, Engine::Execution::ValueStorageImpl<V...>> { };

}