#pragma once

#include "Generic/execution/concepts.h"

#include "Generic/virtual_invoke.h"

namespace Engine {
namespace Execution {

    template <typename R, typename VPack, auto... cpo>
    struct VirtualReceiverBaseEx;

    template <typename... V>
    struct VirtualReceiverBaseEx<type_pack<>, type_pack<V...>> {
        ~VirtualReceiverBaseEx() = default;

        using value_types = type_pack<V...>;
        using result_types = type_pack<>;
        using mapped_cpos = auto_pack<>;

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
    struct VirtualReceiverBaseEx<RPack, VPack, cpo, cpos...> : VirtualCPOsBase<VirtualReceiverBaseEx<RPack, VPack>, cpo, cpos...> {
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

    template <typename Rec, typename Base>
    using VirtualState = VirtualCPOsImpl<VirtualStateEx<Rec, Base, typename Base::result_types, typename Base::value_types>>;

}
}