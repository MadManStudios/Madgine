#pragma once

#include "Generic/type_pack.h"
#include "Generic/auto_pack.h"

#include "Generic/execution/concepts.h"

namespace Engine {
namespace Execution {

    template <typename R, typename VPack, auto... cpo>
    struct VirtualReceiverBaseEx;

    template <typename... V>
    struct VirtualReceiverBaseEx<type_pack<>, type_pack<V...>> {
        ~VirtualReceiverBaseEx() = default;

        using value_types = type_pack<V...>;
        using cpos = auto_pack<>;

    public:
        virtual void set_done() = 0;
        virtual void set_value(V... v) = 0;
    };

    template <typename R, typename... ExtraR, typename VPack>
    struct VirtualReceiverBaseEx<type_pack<R, ExtraR...>, VPack> : VirtualReceiverBaseEx<type_pack<ExtraR...>, VPack> {
        using result_types = type_pack<R, ExtraR...>;

        virtual void set_error(R r) = 0;
    };

    template <typename RPack, typename VPack, auto cpo, auto... cpos>
    struct VirtualReceiverBaseEx<RPack, VPack, cpo, cpos...> : VirtualCPOBase<cpo, VirtualReceiverBaseEx<RPack, VPack, cpos...>> {
        using cpos = auto_pack<cpo, cpos...>;
    };

    template <typename R, typename... V>
    using VirtualReceiverBase = VirtualReceiverBaseEx<make_type_pack_t<R>, type_pack<V...>>;

    template <typename Rec, typename Base, typename R, typename VPack, auto... cpo>
    struct VirtualStateEx;

    template <typename _Rec, typename Base, typename... V>
    struct VirtualStateEx<_Rec, Base, type_pack<>, type_pack<V...>> : Base {

        using Rec = _Rec;

        template <typename... Args>
        VirtualStateEx(Rec &&rec, Args &&...args)
            : Base(std::forward<Args>(args)...)
            , mRec(std::forward<Rec>(rec))
        {
        }
        virtual void set_done() override
        {
            mRec.set_done();
        }
        virtual void set_value(V... v) override
        {
            this->mRec.set_value(std::forward<V>(v)...);
        }

        friend Rec &tag_invoke(Execution::get_receiver_t, VirtualStateEx &state)
        {
            return state.mRec;
        }

        Rec mRec;
    };

    template <typename Rec, typename Base, typename R, typename... ExtraR, typename VPack>
    struct VirtualStateEx<Rec, Base, type_pack<R, ExtraR...>, VPack> : VirtualStateEx<Rec, Base, type_pack<ExtraR...>, VPack> {

        using VirtualStateEx<Rec, Base, type_pack<ExtraR...>, VPack>::VirtualStateEx;

        using result_type = R;

        virtual void set_error(R r) override
        {
            this->mRec.set_error(std::forward<R>(r));
        }
    };

    template <typename Rec, typename Base, typename RPack, typename VPack, auto cpo, auto... cpos>
    struct VirtualStateEx<Rec, Base, RPack, VPack, cpo, cpos...> : VirtualCPOImpl<cpo, Execution::get_receiver, VirtualStateEx<Rec, Base, RPack, VPack, cpos...>> {
        using VirtualCPOImpl<cpo, Execution::get_receiver, VirtualStateEx<Rec, Base, RPack, VPack, cpos...>>::VirtualCPOImpl;
    };

    template <typename Rec, typename Base, typename RPack, typename VPack>
    struct VirtualStateHelper : Base {
        using Base::Base;

        template <auto... cpos>
        using type = VirtualStateEx<Rec, Base, RPack, VPack, cpos...>;
    };

    template <typename Rec, typename Base>
    using VirtualState = typename Base::cpos::instantiate<VirtualStateHelper<Rec, Base, typename Base::result_types, typename Base::value_types>::template type>;

}
}