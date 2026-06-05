#include "../metalib.h"

#include "flags.h"

#include "Generic/enum.h"

namespace Engine {
namespace Reflect {

    bool Flags::operator==(const Flags &other) const
    {
        if (mTable != other.mTable)
            return false;
        if (!mTable)
            return true;
        return mValue == other.mValue;
    }

    const EnumMetaTable *Flags::table() const
    {
        return mTable;
    }

    std::ostream &operator<<(std::ostream &stream, const Flags &value)
    {
        return value.mTable->printFlags(stream, value.mValue);
    }

    std::istream &operator>>(std::istream &stream, Flags &value)
    {
        return value.mTable->readFlags(stream, value.mValue);
    }

}
}