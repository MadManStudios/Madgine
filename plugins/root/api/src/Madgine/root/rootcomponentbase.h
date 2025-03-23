#pragma once

#include "Meta/keyvalue/virtualscope.h"

namespace Engine {
namespace Root {
    struct MADGINE_ROOT_EXPORT RootComponentBase : VirtualScopeBase<> {
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

REGISTER_TYPE(Engine::Root::RootComponentBase)