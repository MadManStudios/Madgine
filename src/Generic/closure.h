#pragma once

namespace Engine {

namespace __generic_impl__ {
    template <typename R, typename... Args>
    struct ClosureWrapper {
        virtual ~ClosureWrapper() = default;
        virtual R call(Args... args) = 0;
    };

    template <typename F, typename R, typename... Args>
    struct ClosureWrapperImpl : ClosureWrapper<R, Args...> {

        ClosureWrapperImpl(F f)
            : mF(std::move(f))
        {
        }

        virtual R call(Args... args) override
        {
            return mF(std::forward<Args>(args)...);
        }

        F mF;
    };

    template <typename R, std::same_as<void> T, typename... Args>
    struct ClosureImpl {

        ClosureImpl() = default;

        template <DecayedNoneOf<ClosureImpl> F>
        requires requires(F& f) {
            f(std::declval<Args>()...);
        }
        ClosureImpl(F &&f)
            : mWrapper(std::make_unique<ClosureWrapperImpl<std::remove_reference_t<F>, R, Args...>>(std::forward<F>(f)))
        {
        }

        template <typename R2, typename T2, typename... Args2>
        ClosureImpl(ClosureImpl<R2, T2, Args2...> &&f)
            : mWrapper(f ? std::make_unique<ClosureWrapperImpl<ClosureImpl<R2, T2, Args2...>, R, Args...>>(std::move(f)) : nullptr)
        {
        }

        ClosureImpl(const ClosureImpl &) = delete;
        ClosureImpl(ClosureImpl &&) = default;

        ClosureImpl &operator=(ClosureImpl &&) = default;

        R operator()(Args... args) const
        {
            return mWrapper->call(std::forward<Args>(args)...);
        }

        explicit operator bool() const
        {
            return static_cast<bool>(mWrapper);
        }

    private:
        std::unique_ptr<ClosureWrapper<R, Args...>> mWrapper;
    };

}


template <typename F>
using Closure = typename CallableTraits<F>::template instance<__generic_impl__::ClosureImpl>;

}