#pragma once

namespace Engine {

template <typename R, std::same_as<void> T, typename... Args>
struct SignatureEntry {
    template <typename F>
    SignatureEntry(F &&f)
        : mF([](void *context, Args &&...args) -> R {
            return std::invoke(*static_cast<std::remove_reference_t<F> *>(context), std::forward<Args>(args)...);
        })
    {
    }

    R operator()(void *context, Args &&...args)
    {
        return mF(context, std::forward<Args>(args)...);
    }

    R (*mF)(void *, Args &&...) = nullptr;
};

template <typename... Signatures>
struct VTable;

template <>
struct VTable<> {
    template <typename F>
    VTable(F &&) { }

    void operator()() = delete;
};

template <typename Signature, typename... Signatures>
struct VTable<Signature, Signatures...> : VTable<Signatures...>, CallableTraits<Signature>::template instance<SignatureEntry> {
    template <typename F>
    VTable(F &&f)
        : VTable<Signatures...>(f)
        , CallableTraits<Signature>::template instance<SignatureEntry>(f)
    {
    }

    using VTable<Signatures...>::operator();
    using CallableTraits<Signature>::template instance<SignatureEntry>::operator();
};

template <typename... Signatures>
struct CallableView {

    template <typename F>
    static VTable<Signatures...> *table(F &&f)
    {
        static VTable<Signatures...> table = f;
        return &table;
    }

    CallableView() = default;

    template <Concepts::DecayedNoneOf<CallableView<Signatures...>> F>
    CallableView(F &&f)
        : mTable(table(std::forward<F>(f)))
        , mContext(&f)
    {
    }

    template <Concepts::DecayedNoneOf<CallableView<Signatures...>> F>
    CallableView &operator=(F &f)
    {
        mTable = table(f);
        mContext = &f;
        return *this;
    }

    template <typename... Args>
    decltype(auto) operator()(Args &&...args) const
    {
        return (*mTable)(mContext, std::forward<Args>(args)...);
    }

private:
    VTable<Signatures...> *mTable = nullptr;
    void *mContext = nullptr;
};

template <typename R, std::same_as<void> T, typename... Args>
struct CallableViewImpl {

    CallableViewImpl() = default;

    template <Concepts::DecayedNoneOf<CallableViewImpl<R, T, Args...>> F>
    CallableViewImpl(F &&f)
        : mEntry(std::forward<F>(f))
        , mContext(&f)
    {
    }

    template <Concepts::DecayedNoneOf<CallableViewImpl<R, T, Args...>> F>
    CallableViewImpl &operator=(F &f)
    {
        mEntry = f;
        mContext = &f;
        return *this;
    }

    R operator()(Args &&...args)
    {
        return mEntry(mContext, std::forward<Args>(args)...);
    }

private:
    SignatureEntry<R, T, Args...> mEntry;
    void *mContext = nullptr;
};

template <typename Signature>
struct CallableView<Signature> : CallableTraits<Signature>::template instance<CallableViewImpl> {
    using CallableTraits<Signature>::template instance<CallableViewImpl>::instance;
};

}