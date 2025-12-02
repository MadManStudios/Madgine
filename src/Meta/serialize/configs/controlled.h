#pragma once

#include "../operations.h"
#include "../helper/typedobjectserialize.h"

namespace Engine {
namespace Serialize {

    struct CreatorCategory;

    template <typename Cmp, auto staticTypeResolve = nullptr>
    struct ControlledConfig {

        using Category = CreatorCategory;

        static const constexpr bool controlled = true;

        template <typename C>
        static void writeItem(CallerHierarchyFormattedSerializeStream out, const std::ranges::range_value_t<C> &t)
        {
            const char *name = "Item";
            
            if (true) {
                name = beginExtendedTypedWrite(out, comparator_traits<Cmp>::to_cmp_type(t));
            } else {
                out.mStream.beginExtendedWrite(name, 1);
                write(out, comparator_traits<Cmp>::to_cmp_type(t), "key");
            }
            write(out, t, name);
        }

        template <typename Op>
        static StreamResult readItem(CallerHierarchyFormattedSerializeStream in, Op &op, std::ranges::iterator_t<Op> &it)
        {
            /*if (it != physical(op).end())
                return STREAM_ERROR(in, StreamState::UNKNOWN_ERROR, "Reading currently only supported at end()");*/

            MakeOwning_t<typename comparator_traits<Cmp>::type> key;
            if (true) {
                STREAM_PROPAGATE_ERROR(beginExtendedTypedRead(in, key));
            } else {
                STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead("Item", 1));

                STREAM_PROPAGATE_ERROR(read(in, key, "key"));
            }
            
            it = std::ranges::find(physical(op), key, &comparator_traits<Cmp>::to_cmp_type);
            if (it == physical(op).end())
                return STREAM_UNKNOWN_ERROR(in) << "Missing item of name '" << key << "' in controlled container";

            return read(in, *it, nullptr);
        }

        template <typename C>
        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const StreamVisitor &visitor, size_t depth)
        {
            MakeOwning_t<typename comparator_traits<Cmp>::type> key;
            if (true) {
                STREAM_PROPAGATE_ERROR(beginExtendedTypedRead(in, key));
            } else {
                STREAM_PROPAGATE_ERROR(in.mStream.beginExtendedRead("Item", 1));

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

        template <typename Op>
        static void clear(Op &op)
        {
            /*if (op.size() < expected) //TODO: ?
                std::terminate();*/
        }
    };

}
}