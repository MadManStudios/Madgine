#pragma once

#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

#include "python3debugger.h"

#include "Modules/threading/madgineobject.h"

namespace Engine {
namespace Behavior {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT Python3Environment : Root::RootComponent<Python3Environment>, Threading::MadgineObject<Python3Environment> {
            Python3Environment(Root::Root &root);            

            Threading::Task<bool> init();
            Threading::Task<void> finalize();

            std::string_view key() const override;

            ExecutionSender execute(std::string_view command);

            static PyGILState_STATE lock();
            static Log::Log *unlock(PyGILState_STATE state);
            static void lock(Log::Log *log, Execution::StopToken st);
            static std::pair<Log::Log *, Execution::StopToken> unlock();

            static size_t totalRefCount();

        private:
            Python3Debugger mDebugger;
        };

    }
}
}