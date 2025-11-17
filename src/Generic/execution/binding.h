#pragma once

#include "../callable_view.h"
#include "concepts.h"

namespace Engine {
namespace Execution {

    struct BindingError {
    };

    template <typename F, typename This>
    struct MemberFunctionBinding;

    template <typename T>
    struct ConstantBinding {

        using type = T;

        ConstantBinding(T &&value)
            : mValue(std::forward<T>(value))
        {
        }

        template <typename P>
        auto operator->*(P &&right) &&
        {
            return MemberFunctionBinding<P, ConstantBinding<T>>({ std::forward<P>(right) }, std::move(*this));
        }

        template <typename P>
        auto operator->*(P &&right) const &
        {
            return MemberFunctionBinding<P, ConstantBinding<T>>({ std::forward<P>(right) }, *this);
        }

        template <std::invocable<const T &> F>
        friend bool tag_invoke(access_binding_t, ConstantBinding &&binding, F &&callback)
        {
            return patch_void(std::forward<F>(callback))(std::move(binding).mValue);            
        }

        template <std::invocable<const T &> F>
        friend bool tag_invoke(access_binding_t, const ConstantBinding &binding, F &&callback)
        {
            return patch_void(std::forward<F>(callback), true)(binding.mValue);            
        }

        T mValue;
    };

    template <typename T>
    ConstantBinding(T &&value) -> ConstantBinding<T>;

    template <typename F, typename... Args>
    struct CallBinding {

        using type = std::invoke_result_t<F, typename std::remove_reference_t<Args>::type...>;

        template <typename... Args2>
        CallBinding(F &&f, Args2 &&...args)
            : mF(std::forward<F>(f))
            , mArgs(std::forward<Args2>(args)...)
        {
        }

        template <typename P>
        auto operator->*(P &&right) &&
        {
            return MemberFunctionBinding<P, CallBinding<F, Args...>>({ std::forward<P>(right) }, std::move(*this));
        }

        template <typename P>
        auto operator->*(P &&right) const &
        {
            return MemberFunctionBinding<P, CallBinding<F, Args...>>({ std::forward<P>(right) }, *this);
        }

        template <size_t I, typename... BoundArgs>
        bool call(auto &&callback, BoundArgs &&...args) &&
        {
            if constexpr (I == sizeof...(Args)) {
                if constexpr (std::same_as<type, void>) {
                    std::invoke(std::move(mF), std::forward<BoundArgs>(args)...);
                    return std::forward<decltype(callback)>(callback)();
                } else {
                    return std::forward<decltype(callback)>(callback)(std::invoke(std::move(mF), std::forward<BoundArgs>(args)...));
                }
            } else {
                return access_binding(
                    std::move(std::get<I>(mArgs)),
                    [this, &callback, &args...](auto &&arg) mutable {
                        return std::move(*this).template call<I + 1>(std::forward<decltype(callback)>(callback), std::forward<BoundArgs>(args)..., std::forward<decltype(arg)>(arg));
                    });
            }
        }

        template <size_t I, typename... BoundArgs>
        bool call(auto &&callback, BoundArgs &&...args) const &
        {
            if constexpr (I == sizeof...(Args)) {
                if constexpr (std::same_as<type, void>) {
                    std::invoke(mF, std::forward<BoundArgs>(args)...);
                    return std::forward<decltype(callback)>(callback)();
                } else {
                    return std::forward<decltype(callback)>(callback)(std::invoke(std::move(mF), std::forward<BoundArgs>(args)...));
                }
            } else {
                return access_binding(
                    std::get<I>(mArgs),
                    [this, &callback, &args...](auto &&arg) mutable {
                        return this->template call<I + 1>(std::forward<decltype(callback)>(callback), std::forward<BoundArgs>(args)..., std::forward<decltype(arg)>(arg));
                    });
            }
        }

        friend bool tag_invoke(access_binding_t, CallBinding &&binding, auto &&callback)
        {
            return std::move(binding).template call<0>(patch_void(std::forward<decltype(callback)>(callback), true));
        }

        friend bool tag_invoke(access_binding_t, const CallBinding &binding, auto &&callback)
        {
            return binding.call<0>(patch_void(std::forward<decltype(callback)>(callback), true));
        }

        std::remove_reference_t<F> mF;
        std::tuple<std::remove_reference_t<Args>...> mArgs;
    };

    template <typename F>
    struct FunctionBinding {

        template <typename T>
        using MakeBinding = std::conditional_t<AnyBinding<T>, T, ConstantBinding<T>>;

        template <typename... Args>
        auto operator()(Args &&...args) &&
        {
            return CallBinding<F, MakeBinding<Args>...> { std::forward<F>(mF), std::forward<Args>(args)... };
        }

        template <typename... Args>
        auto operator()(Args &&...args) const &
        {
            return CallBinding<const F &, MakeBinding<Args>...> { mF, std::forward<Args>(args)... };
        }

        std::remove_reference_t<F> mF;
    };

    template <typename F, typename This>
    struct MemberFunctionBinding {

        template <typename... Args>
        auto operator()(Args &&...args) &&
        {
            return std::move(mFunction)(std::move(mThis), std::forward<Args>(args)...);
        }

        template <typename... Args>
        auto operator()(Args &&...args) const &
        {
            return mFunction(mThis, std::forward<Args>(args)...);
        }

        FunctionBinding<F> mFunction;
        std::remove_reference_t<This> mThis;
    };

    template <typename T>
    struct BindingBridgeBase {
        virtual ~BindingBridgeBase() = default;
        virtual bool access(CallableView<bool(const T &)> callback) = 0;
        std::atomic<uint8_t> mRefCount = 0;
    };

    template <typename T, Binding<T> Binding>
    struct BindingBridge : BindingBridgeBase<T> {
        BindingBridge(Binding &&binding)
            : mBinding(std::move(binding))
        {
        }

        bool access(CallableView<bool(const T &)> callback) override
        {
            return access_binding(mBinding, callback);
        }

        std::remove_reference_t<Binding> mBinding;
    };

    template <typename T>
    struct BindingPtr {

        using type = T;

        BindingPtr() = default;

        template <Binding<T> Binding>
        explicit BindingPtr(Binding &&binding)
            : mPtr(new BindingBridge<T, Binding> { std::forward<Binding>(binding) })
        {
            mPtr->mRefCount.store(1, std::memory_order_relaxed);
        }

        BindingPtr(const BindingPtr &other)
            : mPtr(other.mPtr)
        {
            if (mPtr) {
                mPtr->mRefCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
        BindingPtr(BindingPtr &&other) noexcept
            : mPtr(std::exchange(other.mPtr, nullptr))
        {
        }
        ~BindingPtr()
        {
            if (mPtr && mPtr->mRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                delete mPtr;
            }
        }

        BindingPtr &operator=(const BindingPtr &other)
        {
            if (this != &other) {
                if (mPtr && mPtr->mRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    delete mPtr;
                }
                mPtr = other.mPtr;
                if (mPtr) {
                    mPtr->mRefCount.fetch_add(1, std::memory_order_relaxed);
                }
            }
            return *this;
        }

        BindingPtr &operator=(BindingPtr &&other) noexcept
        {
            if (this != &other) {
                if (mPtr && mPtr->mRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    delete mPtr;
                }
                mPtr = std::exchange(other.mPtr, nullptr);
            }
            return *this;
        }

        template <Binding<T> Binding>
        BindingPtr &operator=(Binding &&binding)
        {
            if (mPtr && mPtr->mRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                delete mPtr;
            }
            mPtr = new BindingBridge<T, Binding> { std::forward<Binding>(binding) };
            mPtr->mRefCount.store(1, std::memory_order_relaxed);
            return *this;
        }

        template <typename F>
        friend bool tag_invoke(access_binding_t, const BindingPtr &ptr, F &&callback)
        {
            if (ptr.mPtr) {
                auto wrapped = patch_void(callback, true);
                return ptr.mPtr->access(CallableView<bool(const T &)> { wrapped });
            } else {
                return false;
            }
        }

        template <typename P>
        auto operator->*(P &&right) &&
        {
            return MemberFunctionBinding<P, BindingPtr<T>>({ std::forward<P>(right) }, std::move(*this));
        }

        template <typename P>
        auto operator->*(P &&right) const &
        {
            return MemberFunctionBinding<P, BindingPtr<T>>({ std::forward<P>(right) }, *this);
        }

        explicit operator bool() const
        {
            return mPtr;
        }

    private:
        BindingBridgeBase<T> *mPtr = nullptr;
    };

    template <AnyBinding B, typename T>
    constexpr bool operator==(B &&binding, T &&value)
    {
        return access_binding(std::forward<B>(binding), [&](auto &&v) {
            return std::forward<T>(value) == std::forward<decltype(v)>(v);
        });
    }

    template <AnyBinding B, typename T>
    constexpr bool operator<(B &&binding, T &&value)
    {
        return access_binding(std::forward<B>(binding), [&](auto &&v) {
            return std::forward<T>(value) > std::forward<decltype(v)>(v);
        });
    }

    template <AnyBinding B, typename T>
    constexpr bool operator>(B &&binding, T &&value)
    {
        return access_binding(std::forward<B>(binding), [&](auto &&v) {
            return std::forward<T>(value) < std::forward<decltype(v)>(v);
        });
    }

}
}