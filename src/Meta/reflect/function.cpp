#include "../metalib.h"

#include "function.h"

namespace Engine {
namespace Reflect {

    Result Function::operator()(Value &retVal, const ArgumentList &args) const
    {
        return mWrapper(mFunction, retVal, args);
    }

}
}