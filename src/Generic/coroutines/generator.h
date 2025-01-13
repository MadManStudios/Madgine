#pragma once

#include "../manuallifetime.h"
#include "handle.h"

namespace Engine {

template <typename T>
struct Generator {

    struct promise_type {

        promise_type() = default;

        Generator get_return_object()
        {
            return { CoroutineHandle<promise_type>::fromPromise(*this) };
        }

        std::suspend_never initial_suspend()
        {
            return {};
        }

        std::suspend_always yield_value(const T &t)
        {
            construct(mValue, t);
            return {};
        }

        void return_void()
        {
        }

        void unhandled_exception() { }

        std::suspend_always final_suspend() noexcept
        {
            return {};
        }

        ManualLifetime<const T &> mValue = std::nullopt;
    };

    Generator(CoroutineHandle<promise_type> handle)
        : mHandle(std::move(handle))
    {
    }    

    Generator(Generator &&) = default;

    ~Generator() noexcept
    {
        if (mHandle && !mHandle.done()) {
            destruct(mHandle->mValue);
        }
    }

    struct iterator {
        using difference_type = std::ptrdiff_t;
        using value_type = std::remove_reference_t<T>;
        using reference_type = const T&;

        iterator(Generator<T> &gen)
            : mGen(&gen)
        {
        }

        struct end_token {
        };

        constexpr bool operator==(const end_token &) const
        {
            return mGen->done();
        }

        iterator &operator++()
        {
            mGen->next();
            return *this;
        }

        void operator++(int)
        {
            mGen->next();
        }

        const T &operator*() const
        {
            return mGen->get();
        }

    private:
        Generator<T> *mGen; //TODO: Store pointer to promise type directly?
    };

    iterator begin()
    {
        return { *this };
    }

    static constexpr typename iterator::end_token end()
    {
        return {};
    }

    const T &get()
    {
        return mHandle->mValue;
    }

    bool next()
    {
        destruct(mHandle->mValue);
        mHandle.resume();
        return !mHandle.done();
    }

    bool done()
    {
        return mHandle.done();
    }

    CoroutineHandle<promise_type> release()
    {
        return std::move(mHandle);
    }

    CoroutineHandle<promise_type> mHandle;
};

}