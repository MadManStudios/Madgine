#pragma once

#include "Modules/threading/madgineobject.h"

#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT Python3Environment : Core::RootComponent<Python3Environment>, Threading::MadgineObject<Python3Environment> {
            Python3Environment(Core::Root &root);

            Threading::Task<bool> init();
            Threading::Task<void> finalize();

            std::string_view key() const override;

            Reflect::Result execute(Reflect::Value &retVal, std::string_view command, Platform::Log::Log *log);

            static size_t totalRefCount();
        };

    }
}
}