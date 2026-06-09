#include "../rootlib.h"

#include "root.h"

#include "Platform/debug/memory/memory.h"
#include "Platform/filesystem/async.h"
#include "Platform/filesystem/path.h"
#include "Platform/log/standardlog.h"
#include "Platform/process/processapi.h"

#include "Modules/plugins/pluginmanager.h"
#include "Modules/threading/awaitables/awaitabletimepoint.h"
#include "Modules/uniquecomponent/uniquecomponentcollectormanager.h"
#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "Madgine/cli/cli.h"
#include "Madgine/cli/parameter.h"

#include "keyvalueregistry.h"
#include "rootcomponentbase.h"

namespace Engine {
namespace Core {

    static Root *sSingleton = nullptr;

#if ENABLE_PLUGINS
    Parameter<bool> noPluginCache { { "--no-plugin-cache", "-npc" }, false, "Disables the loading of the cached plugin selection at startup." };
    Parameter<Platform::Filesystem::Path> loadPlugins { { "--load-plugins", "-lp" }, "", "If set the pluginmanager will load the specified config file after loading the cached plugin-file." };
#endif

    Parameter<bool> toolModeParameter { { "--toolMode", "-t" }, false, "If set, no application will be started. Only the root will be initialized and then immediately shutdown again." };
    Parameter<bool> debugParameter { { "--debug", "-g" }, false, "Marks the build as a debug build in tool mode." };
    Parameter<Platform::Log::MessageType> logLevel { { "--logLevel", "-l" }, Platform::Log::MessageType::DEBUG_TYPE, "Specify log-level." };
    Parameter<Platform::Filesystem::Path> logFile { { "--logFile" }, "out.log", "If set, the log output will be written to the specified path" };

    Root::Root(int argc, char **argv)
        : Root(std::make_unique<CLICore>(argc, argv))
    {
    }

    Root::Root(std::unique_ptr<CLICore> cli)
        : mCLI(std::move(cli))
        , mFileLogListener(*logFile)
        , mTaskQueue("Root")
    {

        assert(!sSingleton);
        sSingleton = this;

        Platform::Log::StandardLog::setLogLevel(logLevel);
        if (!logFile->empty())
            Platform::Log::Log::addListener(&mFileLogListener);

#if ENABLE_PLUGINS
        mPluginManager = std::make_unique<Plugins::PluginManager>();
        mErrorCode = mPluginManager->setup(!noPluginCache, mCLI->mProgramPath.stem(), loadPlugins);
        mCollectorManager = std::make_unique<Plugins::CollectorManager>(*mPluginManager);
#endif

#if ENABLE_MEMTRACKING
        mMemTracker = std::make_unique<Debug::Memory::MemoryTracker>();
#endif

        mComponents = std::make_unique<RootComponentContainer<std::vector<Placeholder<0>>>>(*this);

        for (std::unique_ptr<RootComponentBase> &component : *mComponents) {
            KeyValueRegistry::registerGlobal(component->key().data(), component.get());

            if (mErrorCode == 0)
                mErrorCode = component->mErrorCode;
        }
        if (mErrorCode != 0)
            return;

        mTaskQueue.queueTask(updateAsyncIO());

        mTaskQueue.queue([this]() -> Threading::Task<void> {
            for (std::unique_ptr<RootComponentBase> &component : *mComponents) {
                int result = co_await component->runTools();
                if (mErrorCode == 0)
                    mErrorCode = result;
            }
            if (toolMode())
                mTaskQueue.stop();
        });
    }

    Root::~Root()
    {
        assert(sSingleton == this);
        sSingleton = nullptr;
    }

    Root &Root::getSingleton()
    {
        assert(sSingleton);
        return *sSingleton;
    }

    RootComponentBase &Root::getComponent(size_t i)
    {
        return mComponents->get(i);
    }

    bool Root::toolMode() const
    {
        return toolModeParameter;
    }

    bool Root::debug() const
    {
        if (toolMode()) {
            return debugParameter;
        } else {
#ifndef NDEBUG
            return true;
#else
            return false;
#endif
        }
    }

    Threading::Task<void> Root::updateAsyncIO()
    {
        while (mTaskQueue.running()) {
            Platform::Filesystem::checkAsyncIOCompletion();
            Platform::Process::checkAsyncProcessCompletion();
            co_await 500ms;
        }
        do {
            Platform::Filesystem::cancelAllAsyncIO();
            Platform::Filesystem::checkAsyncIOCompletion();
            Platform::Process::checkAsyncProcessCompletion();
            co_await 0ms;
        } while (Platform::Filesystem::pendingIOOperationCount() > 0 || Platform::Process::pendingProcesses() > 0);
    }

    int Root::errorCode()
    {
        return mErrorCode;
    }

    Threading::TaskQueue *Root::taskQueue()
    {
        return &mTaskQueue;
    }

}
}
