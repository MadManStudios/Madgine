#include "../metalib.h"

#include "enum.h"

#include "Generic/enum.h"

namespace Engine {
namespace Reflect {

    bool Enum::operator==(const Enum &other) const
    {
        if (mTable != other.mTable)
            return false;
        if (!mTable)
            return true;
        return mValue == other.mValue;
    }

    std::string_view Enum::toString() const
    {
        return mTable->toString(mValue);
    }

    const EnumMetaTable *Enum::table() const
    {
        return mTable;
    }

    int32_t Enum::value() const
    {
        return mValue;
    }

    void Enum::setValue(int32_t val)
    {
        assert(mTable);
        assert(mTable->mMin < val && val < mTable->mMax);
        mValue = val;
    }

    std::ostream &operator<<(std::ostream &stream, const Enum &value)
    {
        return value.mTable->print(stream, value.mValue, value.mTable->mName);
    }
    std::istream &operator>>(std::istream &stream, Enum &value)
    {
        return value.mTable->read(stream, value.mValue, value.mTable->mName);
    }

}
}