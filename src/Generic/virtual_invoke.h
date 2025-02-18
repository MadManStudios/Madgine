#pragma once

#include "callable_traits.h"

template <typename CPO_holder, typename Base, typename R, std::same_as<void> T, typename... V>
struct VirtualCPOBaseHelper : Base {
    using Base::Base;

    using CPO = decltype(CPO_holder::value);

    using mapped_cpos = typename Base::mapped_cpos::template append<CPO_holder::value>;

    virtual R v_tag_invoke(CPO _cpo, V... v) = 0;

    template <typename... Args>
    friend auto tag_invoke(CPO _cpo, VirtualCPOBaseHelper &base, Args &&...args)
    {
        return base.v_tag_invoke(_cpo, std::forward<Args>(args)...);
    }
};

template <auto cpo, typename Base>
using VirtualCPOBase = typename Engine::CallableTraits<typename decltype(cpo)::signature>::template instance<VirtualCPOBaseHelper, Engine::auto_holder<cpo>, Base>;

template <typename CPO, typename Base, typename R, std::same_as<void> T, typename... V>
struct VirtualCPOImplHelper : Base {
    using Base::Base;

    virtual R v_tag_invoke(CPO _cpo, V... v) override
    {
        return _cpo(this->mRec, std::forward<V>(v)...);
    }
};

template <auto cpo, typename Base>
using VirtualCPOImpl = typename Engine::CallableTraits<typename decltype(cpo)::signature>::template instance<VirtualCPOImplHelper, decltype(cpo), Base>;

template <auto cpo, typename Base>
using VirtualCPOsImplHelper = VirtualCPOImpl<cpo, Base>;

template <typename Base, auto... cpos>
using VirtualCPOsBase = typename Engine::auto_pack<cpos...>::template fold<VirtualCPOBase, Base>;

template <typename Base>
using VirtualCPOsImpl = typename Base::mapped_cpos::template fold<VirtualCPOsImplHelper, Base>;