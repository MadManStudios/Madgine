#include "../metalib.h"

#include "boundapifunction.h"

#include "Generic/execution/execution.h"

#include "argumentlist.h"
#include "functionargument.h"
#include "functiontable.h"
#include "value.h"

namespace Engine {
namespace Reflect {

    BoundApiFunction::BoundApiFunction(const ApiFunction &f, const ScopePtr &scope)
        : mFunction(f)
        , mScope(scope.mScope)
    {
        assert(f.mTable->mArguments[0].mType.mType == TypeEnum::ScopeValue);
        assert(*f.mTable->mArguments[0].mType.mSecondary.mMetaTable == scope.mType);
        assert(f.mTable->mIsMemberFunction);
    }

    ScopePtr BoundApiFunction::scope() const
    {
        return { mScope, *mFunction.mTable->mArguments[0].mType.mSecondary.mMetaTable };
    }

    Result BoundApiFunction::operator()(Value &retVal, const ArgumentList &args) const
    {
        ArgumentList fullArgs;
        fullArgs.reserve(args.size() + 1);
        fullArgs.push_back(Value { scope() });
        fullArgs.insert(fullArgs.end(), args.begin(), args.end());
        return mFunction(retVal, fullArgs);
    }

}
}