#include "../metalib.h"

#include "nativeobject.h"

#include "objectinstance.h"
#include "valuetype.h"

namespace Engine {

struct NativeObjectInstance : ObjectInstance {
    NativeObjectInstance(std::map<std::string, ValueType, std::less<>> data)
        : mData(std::move(data))
    {
    }

    virtual bool getValue(ValueType &retVal, std::string_view name) const override
    {
        auto it = mData.find(name);
        if (it == mData.end())
            return false;
        retVal = it->second;
        return true;
    }

    virtual void setValue(std::string_view name, const ValueType &value) override
    {
        mData[std::string { name }] = value;
    }

private:
    std::map<std::string, ValueType, std::less<>> mData;
};

NativeObject::NativeObject(std::map<std::string, ValueType, std::less<>> data)
    : ObjectPtr(std::make_shared<NativeObjectInstance>(std::move(data)))
{
}

}