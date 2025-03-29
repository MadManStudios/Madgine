#pragma once

#include "Generic/bits/array.h"

namespace Engine {

struct META_EXPORT FlagsHolder {

    FlagsHolder(const EnumMetaTable *table)
        : mTable(table)
    {
    }

    template <typename Rep>
    FlagsHolder(Flags<Rep> f)
        : mValue(f.values())
        , mTable(&Rep::sTable)
    {
    }

    bool operator==(const FlagsHolder &other) const;

    const EnumMetaTable *table() const;

    decltype(auto) operator[](int32_t i) {
        return mValue[i];
    }

    template <typename T>
    T safe_cast() const
    {
        if (mTable != &T::Representation::sTable)
            throw 0;
        return static_cast<T>(mValue);
    }

    META_EXPORT friend std::ostream &operator<<(std::ostream &stream, const FlagsHolder &value);
    META_EXPORT friend std::istream &operator>>(std::istream &stream, FlagsHolder &value);

private:
    BitArray<64> mValue;
    const EnumMetaTable *mTable;
};

}