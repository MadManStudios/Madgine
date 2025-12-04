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

    template <typename Base, typename Rec, typename R, typename VPack>
    struct VirtualStateEx;

    template <typename Base, typename Rec, typename... V>
    struct VirtualStateEx<Base, Rec, type_pack<>, type_pack<V...>> : Base {

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

    template <typename Base, typename Rec, typename R, typename... ExtraR, typename VPack>
    struct VirtualStateEx<Base, Rec, type_pack<R, ExtraR...>, VPack> : VirtualStateEx<Base, Rec, type_pack<ExtraR...>, VPack> {

        using VirtualStateEx<Base, Rec, type_pack<ExtraR...>, VPack>::VirtualStateEx;

        using result_type = R;

        using VirtualStateEx<Base, Rec, type_pack<ExtraR...>, VPack>::set_error;
        virtual void set_error(R r) override
        {
            this->mRec.set_error(std::forward<R>(r));
        }
    };

    template <typename Base, typename Rec>
    using VirtualState = VirtualCPOsImpl<VirtualStateEx<Base, Rec, typename Base::result_types, typename Base::value_types>>;

}
}