#include "../metalib.h"

#include "nativeobject.h"

#include "objectinstance.h"
#include "value.h"

namespace Engine {
namespace Reflect {

    struct NativeObjectInstance : ObjectInstance {
        NativeObjectInstance(std::map<std::string, Value, std::less<>> data)
            : mData(std::move(data))
        {
        }

        virtual Result getValue(Value &retVal, std::string_view name) const override
        {
            auto it = mData.find(name);
            if (it == mData.end())
                return REFLECT_UNKNOWN_ERROR() << "No field named '" << name << "'!";
            retVal = it->second;
            return {};
        }

        virtual void setValue(std::string_view name, const Value &value) override
        {
            mData[std::string { name }] = value;
        }

    private:
        std::map<std::string, Value, std::less<>> mData;
    };

    NativeObject::NativeObject(std::map<std::string, Value, std::less<>> data)
        : ObjectPtr(std::make_shared<NativeObjectInstance>(std::move(data)))
    {
    }

}
}