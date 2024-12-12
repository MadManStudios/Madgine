#pragma once

#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"
#include "Modules/uniquecomponent/uniquecomponent.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "python3debugger.h"

#include "Generic/execution/virtualsender.h"
#include "Generic/genericresult.h"

#include "Madgine/debug/debuggablesender.h"

#include "Meta/keyvalue/valuetype.h"

#include "util/pyexecution.h"

#include "Modules/threading/madgineobject.h"

namespace Engine {
namespace Scripting {
    namespace Python3 {

        struct MADGINE_PYTHON3_EXPORT Python3Environment : Root::RootComponent<Python3Environment>, Threading::MadgineObject<Python3Environment> {
            Python3Environment(Root::Root &root);            

            Threading::Task<bool> init();
            Threading::Task<void> finalize();

            std::string_view key() const override;

            ExecutionSender execute(std::string_view command);

            static PyGILState_STATE lock();
            static Log::Log *unlock(PyGILState_STATE state);
            static void lock(Log::Log *log, std::stop_token st);
            static std::pair<Log::Log *, std::stop_token> unlock();

            static size_t totalRefCount();

        private:
            Python3Debugger mDebugger;
        };

    }
}
}

REGISTER_TYPE(Engine::Scripting::Python3::Python3Environment)