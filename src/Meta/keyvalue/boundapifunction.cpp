#include "../metalib.h"

#include "boundapifunction.h"
#include "valuetype.h"

#include "functionargument.h"

#include "functiontable.h"

#include "Generic/execution/execution.h"

#include "argumentlist.h"

namespace Engine {

BoundApiFunction::BoundApiFunction(const ApiFunction &f, const ScopePtr &scope)
    : mFunction(f)
    , mScope(scope.mScope)
{
    assert(f.mTable->mArguments[0].mType.mType == ValueTypeEnum::ScopeValue);
    assert(*f.mTable->mArguments[0].mType.mSecondary.mMetaTable == scope.mType);
    assert(f.mTable->mIsMemberFunction);
}

ScopePtr BoundApiFunction::scope() const
{
    return { mScope, *mFunction.mTable->mArguments[0].mType.mSecondary.mMetaTable };
}

void BoundApiFunction::operator()(ValueType &retVal, const ArgumentList &args) const
{
    ArgumentList fullArgs;
    fullArgs.reserve(args.size() + 1);
    fullArgs.push_back(ValueType { scope() });
    fullArgs.insert(fullArgs.end(), args.begin(), args.end());
    return mFunction(retVal, fullArgs);
}

}