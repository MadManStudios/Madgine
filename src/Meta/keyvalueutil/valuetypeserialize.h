#pragma once

#include "../serialize/operations.h"

namespace Engine {
namespace Serialize {

    template <>
    struct META_EXPORT Operations<ValueType> {
        static StreamResult read(FormattedSerializeStream &in, ValueType &v, const char *name, const CallerHierarchyBasePtr &hierarchy = {});
        static void write(FormattedSerializeStream &out, const ValueType &v, const char *name, const CallerHierarchyBasePtr &hierarchy = {});
        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor);     
    };   

    
    template <>
    struct META_EXPORT Operations<ExtendedValueTypeDesc> {
        static StreamResult read(FormattedSerializeStream &in, ExtendedValueTypeDesc &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {});
        static void write(FormattedSerializeStream &out, const ExtendedValueTypeDesc &t, const char *name, const CallerHierarchyBasePtr &hierarchy = {});
        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor);
        static StreamResult applyMap(FormattedSerializeStream &in, ExtendedValueTypeDesc &t, bool success, const CallerHierarchyBasePtr &hierarchy = {});
    };   

    
}
}