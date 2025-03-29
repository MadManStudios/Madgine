#include "metalib.h"

#include "enumholder.h"

#include "Generic/enum.h"

namespace Engine {

bool EnumHolder::operator==(const EnumHolder &other) const
{
    if (mTable != other.mTable)
        return false;
    if (!mTable)
        return true;
    return mValue == other.mValue;
}

std::string_view EnumHolder::toString() const
{
    return mTable->toString(mValue);
}

const EnumMetaTable *EnumHolder::table() const
{
    return mTable;
}

int32_t EnumHolder::value() const
{
    return mValue;
}

void EnumHolder::setValue(int32_t val)
{
    assert(mTable);
    assert(mTable->mMin < val && val < mTable->mMax);
    mValue = val;
}

std::ostream &operator<<(std::ostream &stream, const EnumHolder &value)
{
    return value.mTable->print(stream, value.mValue, value.mTable->mName);
}
std::istream &operator>>(std::istream &stream, EnumHolder &value)
{
    return value.mTable->read(stream, value.mValue, value.mTable->mName);
}

}