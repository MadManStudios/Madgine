#pragma once

#include "flag.h"

namespace Engine {
namespace Execution {

    template <typename R, typename... V>
    struct FutureSharedState {
        Flag<R, V...> mValue;
    };

    template <typename R, typename... V>
    struct Future {

        Future(std::shared_ptr<FutureSharedState<R, V...>> state)
            : mState(std::move(state))
        {
        }

        bool is_value() const
        {
            return mState->mValue.isValue();
        }

        bool is_ready() const
        {
            return mState->mValue.isSet();
        }

        decltype(auto) operator*() const
        {
            return *mState->mValue;
        }

        using is_sender = void;

        using result_type = R;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<V...>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t connect, const Future &future, Rec &&rec)
        {
            struct state {
                using State = tag_invoke_result_t<Execution::connect_t, FlagStub<R, V...> &, Rec>;

                state(std::shared_ptr<FutureSharedState<R, V...>> ptr, Rec &&rec)
                    : mPtr(std::move(ptr))
                    , mState(tag_invoke(Execution::connect_t {}, mPtr->mValue, std::forward<Rec>(rec)))
                {
                }

                void start()
                {
                    mState.start();
                }

                void stop()
                {
                    mState.stop();
                }

                std::shared_ptr<FutureSharedState<R, V...>> mPtr;
                State mState;
            };

            return state { future.mState, std::forward<Rec>(rec) };
        }

    private:
        std::shared_ptr<FutureSharedState<R, V...>> mState;
    };

    template <typename R, typename... V>
    struct Promise {

        Promise()
            : mState(std::make_shared<FutureSharedState<R, V...>>())
        {
        }

        Promise(std::nullopt_t)
        {
        }

        Promise(const Promise &) = delete;
        Promise(Promise &&) = default;

        ~Promise()
        {
            if (mState) {
                if (!mState->mValue.isSet())
                    mState->mValue.set_done();
            }
        }

        Promise &operator=(const Promise &) = delete;
        Promise &operator=(Promise &&) = default;

        Future<R, V...> getFuture() const
        {
            return mState;
        }

        template <typename... V2>
        void set_value(V2 &&...v)
        {
            mState->mValue.set_value(std::forward<V2>(v)...);
        }

        void set_error(R &&r)
        {
            mState->mValue.set_error(std::forward<R>(r));
        }

        void set_done()
        {
            mState->mValue.set_done();
        }

        explicit operator bool() const
        {
            return static_cast<bool>(mState);
        }

    private:
        std::shared_ptr<FutureSharedState<R, V...>> mState;
    };

}
}