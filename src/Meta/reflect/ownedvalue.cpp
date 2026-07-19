#include "../metalib.h"

#include "ownedvalue.h"

#include "../type/storageops.h"
#include "metatable.h"
#include "value.h"

namespace Engine {
namespace Reflect {

    OwnedValue::OwnedValue(std::shared_ptr<Engine::Type::BaseStorage> ptr)
        : mValue(std::move(ptr))
    {
    }

    std::string OwnedValue::name() const
    {
        Value v;
        get(v);
        return v.toShortString();
    }

    bool OwnedValue::operator==(const OwnedValue &other) const
    {
        return mValue == other.mValue;
    }

    void OwnedValue::get(Value &retVal) const
    {
        mValue->toValue(retVal);
    }

    Result OwnedValue::set(const Value &value) const
    {
        return mValue->fromValue(value);
    }

    const MetaTable *OwnedValue::type() const
    {
        return nullptr;
    }

}
}