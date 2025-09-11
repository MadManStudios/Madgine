#pragma once

#include "../callable_view.h"
#include "concepts.h"

namespace Engine {
namespace Execution {

    struct BindingError {
    };

    struct access_binding_t {

        template <typename T, typename F>
        auto operator()(T &&binding, F &&callback) const
            noexcept(is_nothrow_tag_invocable_v<access_binding_t, T, F>)
                -> tag_invoke_result_t<access_binding_t, T, F>
        {
            return tag_invoke(*this, std::forward<T>(binding), std::forward<F>(callback));
        }
    };

    constexpr access_binding_t access_binding {};

    template <typename B, typename T>
    concept Binding = requires(B &&binding) {
        {
            access_binding(binding, [](const T &) { })
        } -> std::same_as<bool>;
    };

    constexpr auto anyBindingCallback = [](const auto &) { };

    template <typename B>
    concept AnyBinding = requires(B &&binding) {
        {
            access_binding(binding, anyBindingCallback)
        } -> std::same_as<bool>;
    };

    template <AnyBinding Binding, typename Rec>
    struct BindingState : Execution::base_state<Rec> {
        BindingState(Binding &&binding, Rec &&rec)
            : Execution::base_state<Rec>(std::forward<Rec>(rec))
            , mBinding(std::forward<Binding>(binding))
        {
        }

        void start()
        {
            if (!access_binding(mBinding, [this](auto &&...v) { this->set_value(std::forward<decltype(v)>(v)...); })) {
                this->set_error(BindingError {});
            }
        }

        void stop()
        {
        }

        Binding mBinding;
    };

    template <AnyBinding Binding, typename Rec>
    auto tag_invoke(connect_t, Binding &&binding, Rec &&rec)
    {
        return BindingState<Binding, Rec> { std::forward<Binding>(binding), std::forward<Rec>(rec) };
    }

    template <typename F, typename This>
    struct MemberFunctionBinding;

    template <typename T>
    struct BindingBase {
        using type = T;

        using is_sender = void;

        using result_type = BindingError;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<std::remove_reference_t<type>>;
    };

    template <>
    struct BindingBase<void> {
        using type = void;

        using is_sender = void;

        using result_type = BindingError;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;
    };

    template <typename T>
    struct BindingBase<T*> {
        using type = T*;        
    };

    template <typename T>
    struct ConstantBinding : BindingBase<T> {        

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
            std::forward<F>(callback)(std::move(binding).mValue);
            return true;
        }

        template <std::invocable<const T &> F>
        friend bool tag_invoke(access_binding_t, const ConstantBinding &binding, F &&callback)
        {
            std::forward<F>(callback)(binding.mValue);
            return true;
        }

        T mValue;
    };

    template <typename F, typename... Args>
    struct CallBinding : BindingBase<std::invoke_result_t<F, typename std::remove_reference_t<Args>::type...>> {

        using type = typename BindingBase<std::invoke_result_t<F, typename std::remove_reference_t<Args>::type...>>::type;

        template <typename... Args2>
        CallBinding(F &&f, Args2&&... args)
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
                    std::forward<decltype(callback)>(callback)();
                } else {
                    std::forward<decltype(callback)>(callback)(std::invoke(std::move(mF), std::forward<BoundArgs>(args)...));
                }
                return true;
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
                    std::forward<decltype(callback)>(callback)();
                } else {
                    std::forward<decltype(callback)>(callback)(std::invoke(std::move(mF), std::forward<BoundArgs>(args)...));
                }
                return true;
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
            return std::move(binding).call<0>(std::forward<decltype(callback)>(callback));
        }

        friend bool tag_invoke(access_binding_t, const CallBinding &binding, auto &&callback)
        {
            return binding.call<0>(std::forward<decltype(callback)>(callback));
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
    struct BindingPtr : BindingBase<T> {

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

        template <typename F>
        friend bool tag_invoke(access_binding_t, const BindingPtr &ptr, F &&callback)
        {
            if (ptr.mPtr) {
                return ptr.mPtr->access(CallableView<bool(const T &)> { callback });
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

    private:
        BindingBridgeBase<T> *mPtr = nullptr;
    };

}
}