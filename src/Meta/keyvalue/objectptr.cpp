#include "../metalib.h"
#include "objectptr.h"
#include "objectinstance.h"
#include "valuetype.h"

namespace Engine {

ObjectPtr::ObjectPtr(std::monostate)
{
}

ObjectPtr::ObjectPtr(const std::shared_ptr<ObjectInstance> &instance)
    : mInstance(instance)
{
}

void ObjectPtr::reset()
{
    mInstance.reset();
}

ObjectInstance *ObjectPtr::get()
{
    return mInstance.get();
}

const ObjectInstance *ObjectPtr::get() const
{
    return mInstance.get();
}

ObjectPtr::operator bool() const
{
    return mInstance.operator bool();
}

void ObjectPtr::setValue(std::string_view name, const ValueType &value)
{
    mInstance->setValue(name, value);
}

bool ObjectPtr::getValue(ValueType &retVal, std::string_view name) const
{
    if (!mInstance)
        return false;
    return mInstance->getValue(retVal, name);
}

std::map<std::string_view, ValueType> ObjectPtr::values() const
{
    return mInstance->values();
}

void ObjectPtr::call(ValueType &retVal, const ArgumentList &args) const
{
    mInstance->call(retVal, args);
}

std::string ObjectPtr::descriptor() const
{
    return mInstance->descriptor();
}

bool ObjectPtr::operator==(const ObjectPtr &other) const
{
    return mInstance == other.mInstance;
}

}
