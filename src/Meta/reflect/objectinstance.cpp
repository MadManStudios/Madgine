#include "../metalib.h"

#include "objectinstance.h"

#include "value.h"

namespace Engine {
namespace Reflect {

    std::map<std::string_view, Value> ObjectInstance::values() const
    {
        return {};
    }

    Result ObjectInstance::call(Value &retVal, const ArgumentList &args)

    {
        throw 0;
    }

    std::string ObjectInstance::descriptor() const
    {
        return "<object>";
    }

}
}