#include "../metalib.h"

#include "apifunction.h"

#include "Generic/execution/execution.h"

#include "functiontable.h"

namespace Engine {
namespace Reflect {

    Result ApiFunction::operator()(Value &retVal, const ArgumentList &args, ContextPtr context) const
    {
        Result result = mTable->mFunctionPtr(mTable, retVal, args, context);
        if (result && result.mError->mStackTrace.empty()) {
            result.mError->mMsg += "\nnote: trying to call " + std::string { mTable->mName };
        }
        return result;
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