#include "../metalib.h"

#include "apifunction.h"

#include "functiontable.h"

#include "Generic/execution/execution.h"

#include "valuetype.h"

namespace Engine {

void ApiFunction::operator()(ValueType &retVal, const ArgumentList &args) const
{
    return mTable->mFunctionPtr(mTable, retVal, args);    
}

size_t ApiFunction::argumentsCount(bool excludeThis) const
{
    return mTable->mArgumentsCount - (excludeThis && mTable->mIsMemberFunction);
}

bool ApiFunction::isMemberFunction() const
{
    return mTable->mIsMemberFunction;
}
}