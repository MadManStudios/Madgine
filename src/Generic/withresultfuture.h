#pragma once

#include "withresult.h"

namespace Engine {

template <typename R, typename T = void>
struct WithResultFuture {

    using type = T;

    WithResultFuture() = default;

    WithResultFuture(WithResultFuture &&) = default;

    WithResultFuture(std::future<WithResult<R, T>> future)
        : mFuture(std::move(future))
    {
    }

    WithResultFuture &operator=(WithResultFuture &&) = default;

    R get(T &result)
    {
        return mFuture.get().get(result);
    }

    operator T()
    {
        return mFuture.get();
    }

    bool is_ready() const
    {
        return mFuture.is_ready();
    }

private:
    std::future<WithResult<R, T>> mFuture;
};

template <typename R>
struct WithResultFuture<R, void> {

    using type = void;

    WithResultFuture() = default;

    WithResultFuture(WithResultFuture &&) = default;

    WithResultFuture(Void v)
        : mFuture(WithResult<R, void> {})
    {
    }

    WithResultFuture(std::future<WithResult<R, void>> future)
        : mFuture(std::move(future))
    {
    }

    WithResultFuture &operator=(WithResultFuture &&) = default;

    R get()
    {
        return mFuture.get().get();
    }

    bool is_ready() const
    {
        return mFuture.is_ready();
    }

private:
    std::future<WithResult<R, void>> mFuture;
};

}