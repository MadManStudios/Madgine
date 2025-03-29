#pragma once

#include "valuetype_desc.h"

namespace Engine {

struct FunctionTable {
    typedef void (*FPtr)(const FunctionTable *, ValueType &, const ArgumentList &);
    FPtr mFunctionPtr;

    std::string_view mName;
    size_t mArgumentsCount;
    bool mIsMemberFunction;
    const FunctionArgument *mArguments;
    ExtendedValueTypeDesc mReturnType = toValueTypeDesc<std::monostate>();

    mutable const FunctionTable *mNext = nullptr;
    mutable const FunctionTable **mPrev = nullptr;
};

}

DLL_IMPORT_VARIABLE(const Engine::FunctionTable, function, auto);

namespace Engine {

META_EXPORT const FunctionTable *&sFunctionList();

META_EXPORT void registerFunction(const FunctionTable &f);
META_EXPORT void unregisterFunction(const FunctionTable &f);

template <auto F>
struct FunctionTableRegistrator {
    FunctionTableRegistrator()
    {
        registerFunction(*function<F>);
    }
    ~FunctionTableRegistrator()
    {
        unregisterFunction(*function<F>);
    }
};

}
