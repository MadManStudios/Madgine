#pragma once

#include "Generic/makeowning.h"

#include "../helper/typedobjectserialize.h"
#include "../operations.h"

namespace Engine {
namespace Serialize {

    struct CreatorCategory;

    template <typename Cmp, auto staticTypeResolve = nullptr>
    struct ControlledConfig {

        using Category = CreatorCategory;

        static const constexpr bool controlled = true;

        template <typename C, typename Context>
        static void writeItem(FormattedSerializeStream &out, const std::ranges::range_value_t<C> &t, Context &&context)
        {
            const char *name = "Item";

            if (true) {
                name = beginExtendedTypedWrite(out, comparator_traits<Cmp>::to_cmp_type(t));
            } else {
                out.beginExtendedWrite(name, 1);
                write(out, comparator_traits<Cmp>::to_cmp_type(t), "key");
            }
            write(out, t, name, context);
        }

        template <typename Op, typename Context>
        static StreamResult readItem(FormattedSerializeStream &in, Op &op, std::ranges::iterator_t<Op> &it, const std::ranges::iterator_t<Op> &where, Context &&context)
        {
            /*if (it != physical(op).end())
                return STREAM_ERROR(in, StreamState::UNKNOWN_ERROR, "Reading currently only supported at end()");*/

            MakeOwning_t<typename comparator_traits<Cmp>::type> key;
            if (true) {
                STREAM_PROPAGATE_ERROR(beginExtendedTypedRead(in, key));
            } else {
                STREAM_PROPAGATE_ERROR(in.beginExtendedRead("Item", 1));

                STREAM_PROPAGATE_ERROR(read(in, key, "key", context));
            }

            it = std::ranges::find(physical(op), key, &comparator_traits<Cmp>::to_cmp_type);
            if (it == physical(op).end())
                return STREAM_UNKNOWN_ERROR(in) << "Missing item of name '" << key << "' in controlled container";

            return read(in, *it, nullptr, context);
        }

        template <typename C>
        static StreamResult visitStream(FormattedSerializeStream &in, const StreamVisitor &visitor, size_t depth)
        {
            MakeOwning_t<typename comparator_traits<Cmp>::type> key;
            if (true) {
                STREAM_PROPAGATE_ERROR(beginExtendedTypedRead(in, key));
            } else {
                STREAM_PROPAGATE_ERROR(in.beginExtendedRead("Item", 1));

                STREAM_PROPAGATE_ERROR(read(in, key, "key"));
            }

            if constexpr (std::same_as<decltype(staticTypeResolve), std::nullptr_t>) {
                using T = std::remove_reference_t<std::ranges::range_reference_t<C>>;
                return Serialize::visitStream<T>(in, nullptr, visitor, depth);
            } else {
                const SerializeTable *type = nullptr;
                STREAM_PROPAGATE_ERROR(staticTypeResolve(type, key));
                assert(type);
                return visitor.visit(PrimitiveHolder<DataTag> { type }, in, nullptr, {}, depth);
            }
        }

        template <typename Op, typename Context>
        static void clear(Op &op, Context &&)
        {
            /*if (op.size() < expected) //TODO: ?
                std::terminate();*/
        }
    };

}
}