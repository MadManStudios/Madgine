#include "../metalib.h"

#include "functiontable.h"

#include "metatable_impl.h"

METATABLE_BEGIN(Engine::FunctionTable)
METATABLE_END(Engine::FunctionTable)

namespace Engine {

const FunctionTable *&sFunctionList()
{
    static const FunctionTable *sDummy = nullptr;
    return sDummy;
}

void registerFunction(const FunctionTable &f)
{
    if (sFunctionList()) {
        sFunctionList()->mPrev = &f.mNext;
    }
    f.mNext = std::exchange(sFunctionList(), &f);
}

void unregisterFunction(const FunctionTable &f)
{
}

}
