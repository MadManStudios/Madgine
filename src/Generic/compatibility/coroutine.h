#pragma once

#if __cpp_lib_coroutine < 201902L
#    if __cpp_lib_coroutine > 0L
#        pragma message "Using fallback for std::coroutines. (__cpp_lib_coroutine: " STRINGIFY2(__cpp_lib_coroutine) ")"
#    else
#        pragma message "Using fallback for std::coroutines. (__cpp_lib_coroutine: undefined)"
#    endif
namespace std {

template <typename R, typename...>
struct coroutine_traits {
    using promise_type = typename R::promise_type;
};

template <typename Promise = void>
struct coroutine_handle;

template <>
struct coroutine_handle<void> {
    static coroutine_handle from_address(void *addr) noexcept
    {
        coroutine_handle me;
        me.ptr = addr;
        return me;
    }
    void operator()() { resume(); }
    void *address() const noexcept { return ptr; }
    void resume() const { __builtin_coro_resume(ptr); }
    void destroy() const { __builtin_coro_destroy(ptr); }
    bool done() const { return __builtin_coro_done(ptr); }
    coroutine_handle &operator=(decltype(nullptr))
    {
        ptr = nullptr;
        return *this;
    }
    coroutine_handle(decltype(nullptr))
        : ptr(nullptr)
    {
    }
    coroutine_handle()
        : ptr(nullptr)
    {
    }
    //  void reset() { ptr = nullptr; } // add to P0057?
    explicit operator bool() const { return ptr; }

protected:
    void *ptr;
};

template <typename Promise>
struct coroutine_handle : coroutine_handle<> {
    using coroutine_handle<>::operator=;

    static coroutine_handle from_address(void *addr) noexcept
    {
        coroutine_handle me;
        me.ptr = addr;
        return me;
    }

    Promise &promise() const
    {
        return *reinterpret_cast<Promise *>(
            __builtin_coro_promise(ptr, alignof(Promise), false));
    }
    static coroutine_handle from_promise(Promise &promise)
    {
        coroutine_handle p;
        p.ptr = __builtin_coro_promise(&promise, alignof(Promise), true);
        return p;
    }
};

template <typename _PromiseT>
bool operator==(coroutine_handle<_PromiseT> const &_Left,
    coroutine_handle<_PromiseT> const &_Right) noexcept
{
    return _Left.address() == _Right.address();
}

template <typename _PromiseT>
bool operator!=(coroutine_handle<_PromiseT> const &_Left,
    coroutine_handle<_PromiseT> const &_Right) noexcept
{
    return !(_Left == _Right);
}

struct suspend_always {
    bool await_ready() noexcept { return false; }
    void await_suspend(coroutine_handle<>) noexcept { }
    void await_resume() noexcept { }
};
struct suspend_never {
    bool await_ready() noexcept { return true; }
    void await_suspend(coroutine_handle<>) noexcept { }
    void await_resume() noexcept { }
};


namespace experimental {

    template <typename T = void>
    struct coroutine_handle : std::coroutine_handle<T> {
    };
    template <typename T, typename...>
    struct coroutine_traits : std::coroutine_traits<T> {
    };
    using suspend_always = std::suspend_always;
    using suspend_never = std::suspend_never;
}

}
#else
#    include <coroutine>
#endif