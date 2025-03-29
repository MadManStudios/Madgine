#include "../serverlib.h"

#include "serverapibase.h"

#include "server.h"

#include "Meta/keyvalue/metatable_impl.h"


METATABLE_BEGIN(Engine::Server::ServerAPIBase)
METATABLE_END(Engine::Server::ServerAPIBase)

namespace Engine {
namespace Server {
    ServerAPIBase::ServerAPIBase(Server &server)
        : mServer(server)
    {
    }

    Threading::TaskQueue *ServerAPIBase::taskQueue() const
    {
        return mServer.taskQueue();
    }

    Server &ServerAPIBase::server()
    {
        return mServer;
    }

    Threading::Task<bool> ServerAPIBase::init()
    {
        co_return true;
    }

    Threading::Task<void> ServerAPIBase::finalize()
    {
        co_return;
    }

    ServerAPIBase &ServerAPIBase::getServerAPIComponent(size_t i)
    {
        return mServer.getServerAPIComponent(i);
    }

}
}

