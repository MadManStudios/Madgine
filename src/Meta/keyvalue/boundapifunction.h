#pragma once

#include "apifunction.h"
#include "scopeptr.h"


namespace Engine {

struct META_EXPORT BoundApiFunction {

    constexpr BoundApiFunction() = default;

    BoundApiFunction(const ApiFunction &f, const ScopePtr &scope);

    bool operator==(const BoundApiFunction &other) const
    {
        return mFunction == other.mFunction && mScope == other.mScope;
    }

    uint32_t argumentsCount(bool excludeThis = false) const
    {
        return mFunction.argumentsCount(excludeThis);
    }

    bool isMemberFunction() const
    {
        return mFunction.isMemberFunction();
    }

    ScopePtr scope() const;

    void operator()(ValueType &retVal, const ArgumentList &args) const;

    ApiFunction mFunction;
    void *mScope = nullptr;
};

template <auto f>
struct TypedBoundApiFunction : TypedApiFunction<f> {

    constexpr TypedBoundApiFunction() = default;

    TypedBoundApiFunction(const ScopePtr &scope)
        : mScope(scope.mScope)
    {
        /* assert(f->mArguments[0].mType.mType == ValueTypeEnum::ScopeValue);
        assert(*f->mArguments[0].mType.mSecondary.mMetaTable == scope.mType);
        assert(f->mIsMemberFunction);*/
    }

    bool operator==(const TypedBoundApiFunction<f> &other) const
    {
        return mScope == other.mScope;
    }

    operator BoundApiFunction() const {
        return { *f, { mScope, *(*f)->mArguments[0].mType.mSecondary.mMetaTable } };
    }

    void *mScope = nullptr;
};

}