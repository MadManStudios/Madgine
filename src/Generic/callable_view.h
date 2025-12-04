#pragma once

namespace Engine {

template <typename R, std::same_as<void> T, typename... Args>
struct CallableViewImpl {

    CallableViewImpl() = default;

    template <typename F>
    CallableViewImpl(F &&f)
        : mF([](void *context, Args &&...args) -> R {
            return std::invoke(*static_cast<std::remove_reference_t<F> *>(context), std::forward<Args>(args)...);
        })
        , mContext(&f)
    {
    }

    template <typename F>
    CallableViewImpl &operator=(F &f)
    {
        mF = [](void *context, Args &&...args) -> R {
            return std::invoke(*static_cast<F *>(context), std::forward<Args>(args)...);
        };
        mContext = &f;
        return *this;
    }

    R operator()(Args &&...args) const
    {
        return mF(mContext, std::forward<Args>(args)...);
    }

private:
    R(*mF)
    (void *, Args &&...) = nullptr;
    void *mContext = nullptr;
};

template <typename Signature>
using CallableView = typename CallableTraits<Signature>::template instance<CallableViewImpl>;

}