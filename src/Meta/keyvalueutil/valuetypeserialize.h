#pragma once

#include "../serialize/operations.h"

namespace Engine {
namespace Serialize {

    template <>
    struct META_EXPORT Operations<ValueType> {
        static StreamResult read(CallerHierarchyFormattedSerializeStream in, ValueType &v, const char *name);
        static void write(CallerHierarchyFormattedSerializeStream out, const ValueType &v, const char *name);
        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth);     
    };   

    
    template <>
    struct META_EXPORT Operations<ExtendedValueTypeDesc> {
        static StreamResult read(CallerHierarchyFormattedSerializeStream in, ExtendedValueTypeDesc &t, const char *name);
        static void write(CallerHierarchyFormattedSerializeStream out, const ExtendedValueTypeDesc &t, const char *name);
        static StreamResult visitStream(CallerHierarchyFormattedSerializeStream in, const char *name, const StreamVisitor &visitor, size_t depth);
    };   

    inline StreamResult tag_invoke(apply_map_t, ExtendedValueTypeDesc &, CallerHierarchyFormattedSerializeStream, bool)
    {
        return {};
    }
    
}
}