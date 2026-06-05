#pragma once

#include "stoppable.h"
#include "storage.h"

namespace Engine {
namespace Execution {

    template <typename Sender, typename Context, typename Handle = std::coroutine_handle<Context>>
    struct AwaitableSender;

    template <typename Sender, typename Context, typename Handle>
    struct AwaitableReceiver {

        template <typename... V>
        void set_value(V &&...value)
        {
            mState.set_value(std::forward<V>(value)...);
        }

        void set_done()
        {
            mState.set_done();
        }

        template <typename... R>
        void set_error(R &&...result)
        {
            mState.set_error(std::forward<R>(result)...);
        }

        template <typename CPO, typename... Args>
            requires(is_tag_invocable_v<CPO, Context &, Args...>)
        friend auto tag_invoke(CPO f, AwaitableReceiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, Context &, Args...>)
            -> tag_invoke_result_t<CPO, Context &, Args...>
        {
            return tag_invoke(f, rec.mContext, std::forward<Args>(args)...);
        }

        AwaitableSender<Sender, Context, Handle> &mState;
        Context &mContext;
    };

    template <typename Sender, typename Context, typename Handle>
    struct AwaitableSender {

        static auto buildState(AwaitableSender &self, Sender &&sender, Context &context)
        {
            return Execution::connect(std::forward<Sender>(sender) | Execution::stoppable, AwaitableReceiver<Sender, Context, Handle> { self, context });
        }

        using S = std::invoke_result_t<decltype(&AwaitableSender::buildState), AwaitableSender &, Sender, Context &>;

        AwaitableSender(Sender &&sender, Context &context)
            : mState(buildState(*this, std::forward<Sender>(sender), context))
        {
        }

        bool await_ready()
        {
            mState.start();
            if (!mFlag.test())
                return false;

            static_assert(!std::same_as<decltype(mResult.error().reproduce(mHandle.promise())), bool>);
            if (mResult.is_done()) {
                return !decltype(mResult.error().reproduce(mHandle.promise())) {};
            } else if (mResult.is_error()) {
                return !decltype(mResult.done().reproduce(mHandle.promise())) {};
            } else {
                return !decltype(mResult.value().reproduce(mHandle.promise())) {};
            }
        }

        template <typename T>
            requires(Concepts::InstanceOf<Handle, std::coroutine_handle>)
        bool await_suspend(std::coroutine_handle<T> handle)
        {
            return await_suspend(Handle::from_promise(handle.promise()));
        }

        bool await_suspend(Handle handle)
        {
            mHandle = std::move(handle);
            if (mFlag.test_and_set()) {
                return mResult.reproduce(mHandle.promise());
            }
            return true;
        }

        decltype(auto) await_resume()
        {
            return Context::unpack_storage(std::move(mResult));
        }

        template <typename... V>
        void set_value(V &&...v)
        {
            mResult.set_value(std::forward<V>(v)...);
            if (mFlag.test_and_set())
                if (!mResult.reproduce(mHandle.promise()))
                    mHandle.resume();
        }

        void set_done()
        {
            mResult.set_done();
            if (mFlag.test_and_set())
                if (!mResult.reproduce(mHandle.promise()))
                    mHandle.resume();
        }

        template <typename... R>
        void set_error(R &&...error)
        {
            mResult.set_error(std::forward<R>(error)...);
            if (mFlag.test_and_set())
                if (!mResult.reproduce(mHandle.promise()))
                    mHandle.resume();
        }

    private:
        S mState;
        std::atomic_flag mFlag = ATOMIC_FLAG_INIT;
        Handle mHandle;
        Execution::ResultStorage<Sender> mResult;
    };

}
}