#pragma once

#include "Meta/reflect/virtualscope.h"

namespace Engine {
namespace Core {
    struct MADGINE_ROOT_EXPORT RootComponentBase : Reflect::VirtualScopeBase<> {
        RootComponentBase(Root &root);
        virtual ~RootComponentBase() = default;

        virtual std::string_view key() const = 0;

        virtual Threading::Task<int> runTools();

        int mErrorCode = 0;

    protected:
        Root &mRoot;
    };
}
}