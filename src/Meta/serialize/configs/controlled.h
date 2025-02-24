#pragma once

#include "../operations.h"

namespace Engine {
namespace Serialize {

    struct CreatorCategory;

    template <typename Cmp, auto staticTypeResolve = nullptr>
    struct ControlledConfig {

        using Category = CreatorCategory;

        static const constexpr bool controlled = true;

        template <typename C>
        static void writeItem(FormattedSerializeStream &out, const std::ranges::range_value_t<C> &t)
        {
            out.beginExtendedWrite("Item", 1);
            write(out, comparator_traits<Cmp>::to_cmp_type(t), "key");
            write(out, t, "Item");
        }

        template <typename Op>
        static StreamResult readItem(FormattedSerializeStream &in, Op &op, std::ranges::iterator_t<Op> &it)
        {
            /*if (it != physical(op).end())
                return STREAM_ERROR(in, StreamState::UNKNOWN_ERROR, "Reading currently only supported at end()");*/

            STREAM_PROPAGATE_ERROR(in.beginExtendedRead("Item", 1));
            MakeOwning_t<typename comparator_traits<Cmp>::type> key;
            STREAM_PROPAGATE_ERROR(read(in, key, "key"));
            it = std::ranges::find(physical(op), key, &comparator_traits<Cmp>::to_cmp_type);
            if (it == physical(op).end())
                return STREAM_UNKNOWN_ERROR(in) << "Missing item of name '" << key << "' in controlled container";

            return read(in, *it, "Item");
        }

        template <typename C>
        static StreamResult visitStream(FormattedSerializeStream &in, const StreamVisitor &visitor)
        {
            STREAM_PROPAGATE_ERROR(in.beginExtendedRead("Item", 1));
            MakeOwning_t<typename comparator_traits<Cmp>::type> key;
            STREAM_PROPAGATE_ERROR(read(in, key, "key"));
            if constexpr (std::same_as<decltype(staticTypeResolve), std::nullptr_t>) {
                using T = std::remove_reference_t<std::ranges::range_reference_t<C>>;
                return Serialize::visitStream<T>(in, "Item", visitor);            
            } else {
                const SerializeTable *type = nullptr;
                STREAM_PROPAGATE_ERROR(staticTypeResolve(type, key));
                assert(type);
                return visitor.visit(PrimitiveHolder<DataTag> { type }, in, "Item", {});                
            }            
        }

        template <typename Op>
        static void clear(Op &op)
        {
            /*if (op.size() < expected) //TODO: ?
                std::terminate();*/
        }
    };

}
}