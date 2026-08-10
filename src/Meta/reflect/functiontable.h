#pragma once

#include "result.h"
#include "type.h"
#include "context.h"

namespace Engine {
namespace Reflect {

    struct FunctionTable {
        typedef Result (*FPtr)(const FunctionTable *, Value &, const ArgumentList &, ContextPtr);
        FPtr mFunctionPtr;

        std::string_view mName;
        size_t mArgumentsCount;
        bool mIsMemberFunction;
        const FunctionArgument *mArguments;
        ExtendedType mReturnType = toType<std::monostate>();

        mutable const FunctionTable *mNext = nullptr;
        mutable const FunctionTable **mPrev = nullptr;
    };

}
}

DLL_IMPORT_VARIABLE(const Engine::Reflect::FunctionTable, function, auto);

namespace Engine {
namespace Reflect {

    META_EXPORT const FunctionTable *&sFunctionList();

    namespace __Reflect_impl__ {

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
}
}