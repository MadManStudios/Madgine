#pragma once

#include "../serialize/operations.h"

namespace Engine {
namespace Serialize {

    template <>
    struct PrimitiveReducer<Reflect::Enum> {
        typedef EnumTag type;
    };

    template <>
    struct PrimitiveReducer<Reflect::Flags> {
        typedef FlagsTag type;
    };


    template <>
    struct META_EXPORT Operations<Reflect::Value> {
        static StreamResult read(CallerHierarchyFormattedSerializeStream in, Reflect::Value &v, const char *name);
        static void write(CallerHierarchyFormattedSerializeStream out, const Reflect::Value &v, const char *name);
        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth);
    };

    template <>
    struct META_EXPORT Operations<Reflect::ExtendedType> {
        static StreamResult read(CallerHierarchyFormattedSerializeStream in, Reflect::ExtendedType &t, const char *name);
        static void write(CallerHierarchyFormattedSerializeStream out, const Reflect::ExtendedType &t, const char *name);
        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth);
    };

    inline StreamResult tag_invoke(apply_map_t, Reflect::ExtendedType &, CallerHierarchyFormattedSerializeStream, bool)
    {
        return {};
    }

    template <typename... Configs>
    inline void tag_invoke(set_active_t<Configs...>, Reflect::ExtendedType &, bool, bool, const CallerHierarchyBasePtr &)
    {
    }

}
}