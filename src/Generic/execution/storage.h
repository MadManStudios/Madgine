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

        void reproduce(auto &rec)
        {
            TupleUnpacker::invokeExpand(LIFT(rec.set_value, &), std::move(mValues));
        }

        std::tuple<V...> mValues;
    };

    template <typename V>
    struct ValueStorageImpl<V> {
        template <typename U>
        ValueStorageImpl(U &&u)
            : mValues(std::forward<U>(u))
        {
        }

        void reproduce(auto &rec)
        {
            rec.set_value(std::forward<V>(std::get<0>(mValues)));
        }

        operator V()
        {
            return std::forward<V>(std::get<0>(mValues));
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

        void reproduce(auto &rec)
        {
            rec.set_error(std::forward<R>(mError));
        }

        R mError;
    };

    template <>
    struct ErrorStorageImpl<void> {
        ErrorStorageImpl() = delete;

        static constexpr bool is_void = true;

        void reproduce(auto &rec)
        {
            throw 0;
        }
    };

    template <typename Sender>
    using ValueStorage = typename std::decay_t<Sender>::template value_types<ValueStorageImpl>;

    template <typename Sender>
    using ErrorStorage = ErrorStorageImpl<typename std::decay_t<Sender>::result_type>;

    struct DoneStorage {
        void reproduce(auto &rec)
        {
            rec.set_done();
        }
    };

    struct NullStorage {
        void reproduce(auto &rec)
        {
            throw 0;
        }
    };

    template <typename Value, typename Error>
    struct ResultStorageImpl {

        static constexpr bool can_have_error = !Error::is_void;

        void reproduce(auto &rec)
        {
            std::visit([&](auto &storage) {
                storage.reproduce(rec);
            },
                mState);
        }

        void reproduce_error(auto &rec)
        {
            std::get<Error>(mState).reproduce(rec);
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

        std::variant<NullStorage, Value, Error, DoneStorage> mState;
    };

    template <typename Sender>
    using ResultStorage = ResultStorageImpl<ValueStorage<Sender>, ErrorStorage<Sender>>;

}
}