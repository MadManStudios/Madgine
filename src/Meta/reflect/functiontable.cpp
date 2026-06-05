#include "../metalib.h"

#include "functiontable.h"

#include "metatable_impl.h"

METATABLE_BEGIN(Engine::Reflect::FunctionTable)
METATABLE_END(Engine::Reflect::FunctionTable)

namespace Engine {
namespace Reflect {

    const FunctionTable *&sFunctionList()
    {
        static const FunctionTable *sDummy = nullptr;
        return sDummy;
    }

    namespace __Reflect_impl__ {

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

}
}
