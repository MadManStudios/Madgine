#pragma once

#include "Generic/containers/bits/array.h"

namespace Engine {
namespace Reflect {

    struct META_EXPORT Flags {

        Flags(const EnumMetaTable *table)
            : mTable(table)
        {
        }

        template <typename Rep>
        Flags(Engine::Flags<Rep> f)
            : mValue(f.value())
            , mTable(&Rep::sTable)
        {
        }

        bool operator==(const Flags &other) const;

        const EnumMetaTable *table() const;

        struct reference {
            reference(uint64_t &value, int32_t index)
                : mValue(value)
                , mIndex(index)
            {
            }
            operator bool() const
            {
                return (mValue & (1ull << mIndex)) != 0;
            }
            reference &operator=(bool value)
            {
                if (value) {
                    mValue |= (1ull << mIndex);
                } else {
                    mValue &= ~(1ull << mIndex);
                }
                return *this;
            }

            uint64_t &mValue;
            uint32_t mIndex;
        };

        reference operator[](int32_t i)
        {
            return { mValue, i };
        }

        template <typename T>
        T safe_cast() const
        {
            if (mTable != &T::Representation::sTable)
                throw 0;
            return static_cast<T::EnumType>(mValue);
        }

        META_EXPORT friend std::ostream &operator<<(std::ostream &stream, const Flags &value);
        META_EXPORT friend std::istream &operator>>(std::istream &stream, Flags &value);

    private:
        uint64_t mValue;
        const EnumMetaTable *mTable;
    };

}
}