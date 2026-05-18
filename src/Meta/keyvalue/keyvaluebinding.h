#pragma once

#include "Generic/callable_view.h"
#include "Generic/execution/binding.h"

#include "keyvalueresult.h"
#include "valuetype_desc.h"

namespace Engine {

META_EXPORT void ValueType_erased(CallableView<void(ValueType &)> cb);

template <typename TypeInfo>
struct KeyValueBindingBase {

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
            ValueType_erased([&](ValueType &v) {
                result = Execution::access_binding(ptr.mBinding, [&](auto &&b) {
                    to_ValueType(v, forward_ref<decltype(b)>(b));
                    std::forward<F>(callback)(v);
                });
            });

            return result;
        }
        std::remove_reference_t<Binding> mBinding;
    };

    KeyValueBindingBase() = default;

    template <DecayedNoneOf<KeyValueBindingBase> Binding>
        requires Execution::AnyBinding<Binding>
    KeyValueBindingBase(Binding &&binding, TypeInfo info)
        : mPtr(InnerBinding<Binding> { std::forward<Binding>(binding) })
        , mType(info)
    {
    }

    template <typename P>
    decltype(auto) operator->*(P &&right) const
    {
        return mPtr->*std::forward<P>(right);
    }

    template <typename F, std::derived_from<KeyValueBindingBase<TypeInfo>> T>
    friend bool tag_invoke(Execution::access_binding_t access, const T &binding, F &&callback)
    {
        return tag_invoke(access, binding.mPtr, std::forward<F>(callback));
    }

    template <typename T>
    struct Unwrapper {

        using type = T::type;

        template <typename F>
        friend bool tag_invoke(Execution::access_binding_t access, const Unwrapper &binding, F &&callback)
        {
            return tag_invoke(access, binding.mPtr, [&](const ValueType &v) {
                KeyValueResult error;
                ValueType_erased([&](ValueType &result) {
                    error = ValueType_unwrap(result, std::forward<F>(callback), v);
                });
                if (error) {
                    throw 0;
                    return false;
                }
                return true;
            });
        }

        Execution::BindingPtr<const ValueType &> mPtr;
    };

    template <typename T>
    Unwrapper<T> unwrap() const
    {
        return { mPtr };
    }

    auto operator<=>(const KeyValueBindingBase &) const = default;
    bool operator==(const KeyValueBindingBase &) const = default;

    Execution::BindingPtr<const ValueType &> mPtr;
    TypeInfo mType;
};

using KeyValueBinding = KeyValueBindingBase<ValueTypeIndex>;

using KeyValueScopeBinding = KeyValueBindingBase<const MetaTable*>;

}