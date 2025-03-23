#include "../serverlib.h"

#include "server.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"

#include "serverapibase.h"

METATABLE_BEGIN(Engine::Server::Server)
METATABLE_END(Engine::Server::Server)

namespace Engine {
namespace Server {
    Server::Server(std::function<int(Closure<void(Engine::App::Application &, Engine::Window::MainWindow &)>)> entrypoint)
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

    void Server::spawnInstance(std::string_view name, Closure<void(Engine::App::Application &, Engine::Window::MainWindow &)> callback)
    {
        mInstances.emplace_back(name, [this, callback { std::move(callback) }]() mutable { return mEntrypoint(std::move(callback)); });
    }

}
}