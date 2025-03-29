#pragma once

namespace Engine {

template <typename R, typename T = void>
struct WithResult {
    WithResult() = default;
    WithResult(T t)
        : mData(std::move(t))
    {
    }
    WithResult(R result, std::optional<T> data = {})
        : mData(std::move(data))
        , mResult(result)
    {
    }

    R get(T &data)
    {
        if (mData)
            data = std::move(*mData);
        return mResult;
    }

    operator T&() &
    {
        return *mData;
    }

    operator T() &&
    {
        return std::move(*mData);
    }

    std::optional<T> mData;
    R mResult;
};

template <typename R>
struct WithResult<R, void> {
    WithResult() = default;
    WithResult(R result)
        : mResult(result)
    {
    }

    R get()
    {
        return mResult;
    }

    R mResult;
};

}