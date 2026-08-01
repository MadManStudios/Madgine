#pragma once

#include "Generic/callable_view.h"
#include "Generic/execution/binding.h"

#include "result.h"
#include "type.h"

namespace Engine {
namespace Reflect {

    META_EXPORT void Value_erased(CallableView<void(Value &)> cb);

    template <typename TypeInfo>
    struct BindingBase {

        template <Execution::AnyBinding Binding>
        struct InnerBinding {
            InnerBinding(Binding &&binding)
                : mBinding(std::forward<Binding>(binding))
            {
            }
            template <typename F>
            friend bool tag_invoke(Execution::access_binding_t, const InnerBinding &ptr, F &&callback)
            {
                bool result;
                Value_erased([&](Value &v) {
                    result = Execution::access_binding(ptr.mBinding, [&](auto &&b) {
                        toValue(v, forward_ref<decltype(b)>(b));
                        std::forward<F>(callback)(v);
                    });
                });

                return result;
            }
            std::remove_reference_t<Binding> mBinding;
        };

        BindingBase() = default;

        template <Concepts::DecayedNoneOf<BindingBase> Binding>
            requires Execution::AnyBinding<Binding>
        BindingBase(Binding &&binding, TypeInfo info)
            : mPtr(InnerBinding<Binding> { std::forward<Binding>(binding) })
            , mType(info)
        {
        }

        template <typename P>
        decltype(auto) operator->*(P &&right) const
        {
            return mPtr->*std::forward<P>(right);
        }

        template <typename F, std::derived_from<BindingBase<TypeInfo>> T>
        friend bool tag_invoke(Execution::access_binding_t access, const T &binding, F &&callback)
        {
            return tag_invoke(access, binding.mPtr, std::forward<F>(callback));
        }

        template <typename T>
        struct Unwrapper {

            using type = T;

            template <typename F>
            friend bool tag_invoke(Execution::access_binding_t access, const Unwrapper &binding, F &&callback)
            {
                return tag_invoke(access, binding.mPtr, [&](const Value &v) {
                    Result error;
                    Value_erased([&](Value &result) {
                        error = invoke_free(result, std::forward<F>(callback), v);
                    });
                    if (error) {
                        throw 0;
                        return false;
                    }
                    return true;
                });
            }

            Execution::BindingPtr<const Value &> mPtr;
        };

        template <typename T>
        Unwrapper<T> unwrap() const
        {
            return { mPtr };
        }

        auto operator<=>(const BindingBase &) const = default;
        bool operator==(const BindingBase &) const = default;

        Execution::BindingPtr<const Value &> mPtr;
        TypeInfo mType;
    };

    using Binding = BindingBase<TypeIndex>;

    using ScopeBinding = BindingBase<const MetaTable *>;

}
}