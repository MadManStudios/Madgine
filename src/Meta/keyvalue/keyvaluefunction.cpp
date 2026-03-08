#include "../metalib.h"

#include "keyvaluefunction.h"

namespace Engine {

KeyValueResult KeyValueFunction::operator()(ValueType &retVal, const ArgumentList &args) const
{
    return mWrapper(mFunction, retVal, args);
}

}