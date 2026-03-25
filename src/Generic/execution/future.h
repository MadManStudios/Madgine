#pragma once

#include "awaitablesender.h"
#include "flag.h"

namespace Engine {
namespace Execution {

    template <typename R, typename... V>
    struct FutureSharedState {
        Flag<R, V...> mValue;
    };

    template <typename R, typename... V>
    struct Future {

        Future() = default;

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

        explicit operator bool() const
        {
            return static_cast<bool>(mState);
        }

        void reset()
        {
            mState.reset();
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

        struct promise_type : FutureSharedState<R, V...> {

            Future<R, V...> get_return_object()
            {
                mState = {
                    this, [](promise_type *p) {
                        std::coroutine_handle<promise_type>::from_promise(*p).destroy();
                    }
                };
                return mState;
            }

            constexpr std::suspend_never initial_suspend() const noexcept
            {
                return {};
            }

            struct FinalSuspend {
                bool await_ready() const noexcept
                {
                    return false;
                }

                void await_suspend(std::coroutine_handle<promise_type> handle) noexcept
                {
                    handle.promise().mState.reset();
                }

                void await_resume() noexcept
                {
                    throw 0;
                }
            };

            constexpr FinalSuspend final_suspend() const noexcept
            {
                return {};
            }

            void return_value(V... value)
            {
                this->mValue.set_value(std::forward<V>(value)...);
            }

            void unhandled_exception()
            {
                throw;
            }

            static decltype(auto) unpack_storage(auto&& storage)
            {
                return std::forward<decltype(storage)>(storage).value().get();
            }

            bool set_value(auto &&...) {
                return false;
            }
            bool set_error(R error) {
                mState->mValue.set_error(std::forward<R>(error));
                return true;
            }
            bool set_done() {
                mState->mValue.set_done();
                return true;
            }

            template <typename T>
            decltype(auto) await_transform(T &&awaitable)
            {
                if constexpr (AnySender<std::remove_reference_t<T>>) {
                    return AwaitableSender<T, promise_type> { std::forward<T>(awaitable), *this };
                } else {
                    return std::forward<T>(awaitable);
                }
            }

            std::shared_ptr<FutureSharedState<R, V...>> mState;
        };

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

        void set_error(patch_void_t<R> &&r)
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