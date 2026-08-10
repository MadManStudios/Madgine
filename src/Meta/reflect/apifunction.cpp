#include "../metalib.h"

#include "apifunction.h"

#include "Generic/execution/execution.h"

#include "functiontable.h"

namespace Engine {
namespace Reflect {

    Result ApiFunction::operator()(Value &retVal, const ArgumentList &args, ContextPtr context) const
    {
        return mTable->mFunctionPtr(mTable, retVal, args, context);
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
}