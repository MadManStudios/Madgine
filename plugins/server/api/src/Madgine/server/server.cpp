#include "../serverlib.h"

#include "server.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "Meta/reflect/metatable_impl.h"

#include "serverapibase.h"

METATABLE_BEGIN(Engine::Core::Server)
METATABLE_END(Engine::Core::Server)

namespace Engine {
namespace Core {
    Server::Server(std::function<int(Closure<void(Application &, MainWindow &)>)> entrypoint)
        : mTaskQueue("Server")
        , mEntrypoint(entrypoint)
        , mServerAPIs(*this)
    {
        mTaskQueue.addSetupSteps(
            [this]() { return callInit(); },
            [this]() { return callFinalize(); });
    }

    Server::~Server()
    {
        mInstances.clear();
    }

    ServerAPIBase &Server::getServerAPIComponent(size_t i)
    {
        return mServerAPIs.get(i);
    }

    Threading::TaskQueue *Server::taskQueue()
    {
        return &mTaskQueue;
    }

    Threading::Task<bool> Server::init()
    {
        co_return true;
    }

    Threading::Task<void> Server::finalize()
    {
        co_return;
    }

    void Server::spawnInstance(std::string_view name, Closure<void(Application &, MainWindow &)> callback)
    {
        mInstances.emplace_back(name, [this, callback { std::move(callback) }]() mutable { return mEntrypoint(std::move(callback)); });
    }

}
}