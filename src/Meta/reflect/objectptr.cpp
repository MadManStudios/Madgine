#include "../metalib.h"

#include "objectptr.h"

#include "objectinstance.h"
#include "value.h"

namespace Engine {
namespace Reflect {

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

    void ObjectPtr::setValue(std::string_view name, const Value &value)
    {
        mInstance->setValue(name, value);
    }

    Result ObjectPtr::getValue(Value &retVal, std::string_view name) const
    {
        if (!mInstance)
            return REFLECT_UNKNOWN_ERROR() << "cannot get value of null reference";
        return mInstance->getValue(retVal, name);
    }

    std::map<std::string_view, Value> ObjectPtr::values() const
    {
        return mInstance->values();
    }

    Result ObjectPtr::call(Value &retVal, const ArgumentList &args) const
    {
        return mInstance->call(retVal, args);
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
}