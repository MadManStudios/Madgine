#pragma once

namespace Engine {

META_EXPORT void ValueType_erased(CallableView<void(ValueType &)> cb);

struct KeyValueBinding {

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
                    return true;
                });
                if (result)
                    std::forward<F>(callback)(v);
            });

            return result;
        }
        std::remove_reference_t<Binding> mBinding;
    };

    template <Execution::AnyBinding Binding>
    KeyValueBinding(Binding &&binding)
        : mPtr(InnerBinding<Binding> { std::forward<Binding>(binding) })
    {
    }

    template <typename P>
    decltype(auto) operator->*(P &&right) const
    {
        return mPtr->*std::forward<P>(right);
    }

    Execution::BindingPtr<const ValueType &> mPtr;
};

}