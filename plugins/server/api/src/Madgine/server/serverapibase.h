#pragma once

#include "Modules/threading/madgineobject.h"

#include "Meta/keyvalue/virtualscope.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

namespace Engine {
namespace Server {
    struct MADGINE_SERVER_EXPORT ServerAPIBase : VirtualScopeBase<>, Threading::MadgineObject<ServerAPIBase> {
        ServerAPIBase(Server &server);
        virtual ~ServerAPIBase() = default;

        template <typename T>
        T &getServerAPIComponent()
        {
            return static_cast<T &>(getServerAPIComponent(UniqueComponent::component_index<T>()));
        }

        ServerAPIBase &getServerAPIComponent(size_t i);

        Server &server();

        Threading::TaskQueue *taskQueue() const;

    protected:
        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();

        friend struct MadgineObject<ServerAPIBase>;

        Server &mServer;
    };
}
}

REGISTER_TYPE(Engine::Server::ServerAPIBase)