#pragma once

#include "type.h"

namespace Engine {
namespace Reflect {

    using AccessorFlags = uint32_t;

    enum AccessorFlags_ {
        AccessorFlags_Named = (1 << 0),
        AccessorFlags_Default = 0
    };

    struct Accessor {
        const char *mName = nullptr;
        bool (*mCheck)(const Accessor *self, const Value &) = nullptr;
        Result (*mGetter)(const Accessor *self, Value &, const Value &) = nullptr;
        Result (*mSetter)(const Accessor *self, const Value &, const Value &) = nullptr;
        ExtendedType mType { ExtendedTypeEnum::GenericType };
        AccessorFlags mFlags = AccessorFlags_Default;
    };

}
}