#include "metalib.h"

#include "flagsholder.h"

#include "Generic/enum.h"

namespace Engine {

bool FlagsHolder::operator==(const FlagsHolder &other) const
{
    if (mTable != other.mTable)
        return false;
    if (!mTable)
        return true;
    return mValue == other.mValue;
}

const EnumMetaTable *FlagsHolder::table() const
{
    return mTable;
}

std::ostream &operator<<(std::ostream &stream, const FlagsHolder &value)
{
    return value.mTable->printFlags(stream, value.mValue);
}

std::istream &operator>>(std::istream &stream, FlagsHolder &value)
{
    return value.mTable->readFlags(stream, value.mValue);
}

}