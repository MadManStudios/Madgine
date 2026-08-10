#pragma once

#include "Generic/callable_view.h"
#include "Generic/execution/binding.h"

#include "result.h"
#include "type.h"
#include "context.h"

namespace Engine {
namespace Reflect {

    META_EXPORT void Value_erased(CallableView<void(Value &)> cb);

    namespace __Reflect_impl__ {

        struct InnerBindingBase {
            virtual ~InnerBindingBase() = default;
            virtual Result access(CallableView<Result(const Value &, ContextPtr)> cb, ContextPtr) const = 0;
            std::atomic<uint8_t> mRefCount = 0;
        };

        template <Execution::AnyBinding Binding>
        struct InnerBinding : InnerBindingBase {
            InnerBinding(Binding &&binding)
                : mBinding(std::forward<Binding>(binding))
            {
            }

            Result access(CallableView<Result(const Value &, ContextPtr)> cb, ContextPtr context) const override
            {
                Result result;
                Value_erased([&](Value &v) {
                    if (!Execution::access_binding(mBinding, [&](auto &&b) {
                            toValue(v, forward_ref<decltype(b)>(b));
                            result = cb(v, context);
                        })) {
                        result = REFLECT_UNKNOWN_ERROR() << "Failed to access binding";
                    }
                });

                return {};
            }

            std::remove_reference_t<Binding> mBinding;
        };

    }

    template <typename TypeInfo>
    struct BindingBase {

        BindingBase() = default;

        template <Concepts::DecayedNoneOf<BindingBase> Binding>
            requires Execution::AnyBinding<Binding>
        BindingBase(Binding &&binding, TypeInfo info)
            : BindingBase(new __Reflect_impl__::InnerBinding<Binding> { std::forward<Binding>(binding) }, info)
        {
        }

        BindingBase(__Reflect_impl__::InnerBindingBase *ptr, TypeInfo info)
            : mPtr(ptr)
            , mType(info)
        {
            mPtr->mRefCount.store(1, std::memory_order_relaxed);
        }

        BindingBase(const BindingBase &other)
            : mPtr(other.mPtr)
            , mType(other.mType)
        {
            if (mPtr) {
                mPtr->mRefCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
        BindingBase(BindingBase &&other) noexcept
            : mPtr(std::exchange(other.mPtr, nullptr))
            , mType(other.mType)
        {
        }
        ~BindingBase()
        {
            if (mPtr && mPtr->mRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                delete mPtr;
            }
        }

        BindingBase &operator=(const BindingBase &other)
        {
            if (this != &other) {
                if (mPtr && mPtr->mRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    delete mPtr;
                }
                mPtr = other.mPtr;
                if (mPtr) {
                    mPtr->mRefCount.fetch_add(1, std::memory_order_relaxed);
                }
                mType = other.mType;
            }
            return *this;
        }

        BindingBase &operator=(BindingBase &&other) noexcept
        {
            if (this != &other) {
                if (mPtr && mPtr->mRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    delete mPtr;
                }
                mPtr = std::exchange(other.mPtr, nullptr);
                mType = other.mType;
            }
            return *this;
        }

        template <typename P>
        decltype(auto) operator->*(P &&right) const
        {
            return mPtr->*std::forward<P>(right);
        }

        template <typename T>
        struct Typed {

            using type = T;

            template <typename F, typename Context = ContextPtr>
            Result access(F &&callback, Context &&context) const
            {
                return mPtr.access([&](const Value &v, ContextPtr context) {
                    return call(std::forward<F>(callback), v, context);
                },
                    context);
            }

            template <typename F>
            friend bool tag_invoke(Execution::access_binding_t access, const Typed &binding, F &&callback)
            {
                return !binding.access([&](const T &v) -> Result {
                    bool result = patch_void(std::forward<F>(callback), true)(v);
                    if (!result)
                        return REFLECT_UNKNOWN_ERROR() << "Failed to access binding";
                    return {};
                },
                    {});
            }

            BindingBase mPtr;
        };

        template <typename T>
        Typed<T> typed() const
        {
            return { *this };
        }

        template <typename F, typename Context = ContextPtr>
        Result access(F &&callback, Context &&context) const
        {
            return mPtr->access(std::forward<F>(callback), context);
        }

        template <typename F>
        friend bool tag_invoke(Execution::access_binding_t access, const BindingBase &binding, F &&callback)
        {
            return !binding.access([&](const Value &v, ContextPtr ignored) -> Result {
                bool result = patch_void(std::forward<F>(callback), true)(v);
                if (!result)
                    return REFLECT_UNKNOWN_ERROR() << "Failed to access binding";
                return {};
            },
                {});
        }

        auto operator<=>(const BindingBase &) const = default;
        bool operator==(const BindingBase &) const = default;

        __Reflect_impl__::InnerBindingBase *mPtr = nullptr;
        TypeInfo mType;
    };

    using Binding = BindingBase<TypeIndex>;

    using ScopeBinding = BindingBase<const MetaTable *>;
}
}