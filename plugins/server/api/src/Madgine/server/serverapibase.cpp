#include "../serverlib.h"

#include "serverapibase.h"

#include "Meta/reflect/metatable_impl.h"

#include "server.h"

METATABLE_BEGIN(Engine::Core::ServerAPIBase)
METATABLE_END(Engine::Core::ServerAPIBase)

namespace Engine {
namespace Core {
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
