#pragma once

#include "Modules/threading/taskqueue.h"
#include "Modules/uniquecomponent/uniquecomponentcontainer.h"
#include "rootcomponentcollector.h"
#include "Interfaces/log/fileloglistener.h"

namespace Engine {
namespace Root {

    struct MADGINE_ROOT_EXPORT Root {
        Root(int argc = 0, char **argv = nullptr);
        Root(std::unique_ptr<CLI::CLICore> cli);
        ~Root();

        int errorCode();

        Threading::TaskQueue *taskQueue();

        template <typename T>
        T &getComponent()
        {
            return static_cast<T &>(getComponent(UniqueComponent::component_index<T>()));
        }

        RootComponentBase &getComponent(size_t i);

        static Root &getSingleton();

        bool toolMode() const;

    protected:
        Threading::Task<void> updateAsyncIO();

    private:
        int mErrorCode = 0;

        std::unique_ptr<CLI::CLICore> mCLI;
        Log::FileLogListener mFileLogListener;

#if ENABLE_PLUGINS
        std::unique_ptr<Plugins::PluginManager> mPluginManager;
        std::unique_ptr<UniqueComponent::CollectorManager> mCollectorManager;
#endif

#if ENABLE_MEMTRACKING
        std::unique_ptr<Debug::Memory::MemoryTracker> mMemTracker;
#endif

        Threading::TaskQueue mTaskQueue;

        std::unique_ptr<RootComponentContainer<std::vector<Placeholder<0>>>> mComponents;
    };

}
}