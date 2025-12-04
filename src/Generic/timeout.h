#pragma once

namespace Engine {

struct TimeOut {
    TimeOut()
        : mEnd(std::chrono::steady_clock::time_point::max())
    {
    }

    template <typename T, typename R>
    TimeOut(std::chrono::duration<T, R> duration)
        : mEnd(duration == std::chrono::duration<T, R>::zero() ? std::chrono::steady_clock::time_point::min() : std::chrono::steady_clock::now() + duration)
    {
    }

    /*TimeOut(std::chrono::steady_clock::time_point point) :
            mEnd(point)
    {}*/

    TimeOut(const TimeOut &other)
        : mEnd(other.mEnd)
    {
    }

    bool expired()
    {
        return std::chrono::steady_clock::now() >= mEnd;
    }

    bool isInfinite()
    {
        return mEnd == std::chrono::steady_clock::time_point::max();
    }

    bool isZero()
    {
        return mEnd == std::chrono::steady_clock::time_point::min();
    }

    std::chrono::steady_clock::duration remainder()
    {
        assert(!isInfinite() && !isZero());
        return mEnd - std::chrono::steady_clock::now();
    }

private:
    std::chrono::steady_clock::time_point mEnd;
};

}