#include "../rootlib.h"

#include "root.h"

#include "Interfaces/debug/memory/memory.h"
#include "Modules/plugins/pluginmanager.h"
#include "keyvalueregistry.h"

#include "Madgine/cli/cli.h"
#include "Modules/uniquecomponent/uniquecomponentcollectormanager.h"

#include "Madgine/cli/parameter.h"

#include "rootcomponentbase.h"

#include "Interfaces/log/standardlog.h"

#include "Interfaces/filesystem/async.h"

#include "Interfaces/filesystem/path.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"

namespace Engine {
namespace Root {

    static Root *sSingleton = nullptr;

#if ENABLE_PLUGINS
    CLI::Parameter<bool> noPluginCache { { "--no-plugin-cache", "-npc" }, false, "Disables the loading of the cached plugin selection at startup." };
    CLI::Parameter<Filesystem::Path> loadPlugins { { "--load-plugins", "-lp" }, "", "If set the pluginmanager will load the specified config file after loading the cached plugin-file." };
#endif

    CLI::Parameter<bool> toolModeParameter { { "--toolMode", "-t" }, false, "If set, no application will be started. Only the root will be initialized and then immediately shutdown again." };
    CLI::Parameter<Engine::Log::MessageType> logLevel { { "--logLevel", "-l" }, Engine::Log::MessageType::DEBUG_TYPE, "Specify log-level." };
    CLI::Parameter<Filesystem::Path> logFile { { "--logFile" }, "out.log", "If set, the log output will be written to the specified path" };

    Root::Root(int argc, char **argv)
        : Root(std::make_unique<CLI::CLICore>(argc, argv))
    {
    }

    Root::Root(std::unique_ptr<CLI::CLICore> cli)
        : mCLI(std::move(cli))
        , mFileLogListener(*logFile)
        , mTaskQueue("Root")
    {

        assert(!sSingleton);
        sSingleton = this;

        Log::StandardLog::setLogLevel(logLevel);
        if (!logFile->empty())
            Log::Log::addListener(&mFileLogListener);

#if ENABLE_PLUGINS
        mPluginManager = std::make_unique<Plugins::PluginManager>();
        mErrorCode = mPluginManager->setup(!noPluginCache, mCLI->mProgramPath.stem(), loadPlugins);
        mCollectorManager = std::make_unique<UniqueComponent::CollectorManager>(*mPluginManager);
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

    Threading::Task<void> Root::updateAsyncIO()
    {
        while (mTaskQueue.running()) {
            Filesystem::checkAsyncIOCompletion();
            co_await 500ms;
        }
        do {
            Filesystem::cancelAllAsyncIO();
            Filesystem::checkAsyncIOCompletion();
            co_await 0ms;
        } while (Filesystem::pendingIOOperationCount() > 0);
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
