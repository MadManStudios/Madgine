#pragma once

#include "Meta/reflect/virtualscope.h"

#include "Modules/threading/madgineobject.h"
#include "Modules/uniquecomponent/uniquecomponent.h"

namespace Engine {
namespace Core {
    struct MADGINE_SERVER_EXPORT ServerAPIBase : Reflect::VirtualScopeBase<>, Threading::MadgineObject<ServerAPIBase> {
        ServerAPIBase(Server &server);
        virtual ~ServerAPIBase() = default;

        template <typename T>
        T &getServerAPIComponent()
        {
            return static_cast<T &>(getServerAPIComponent(Plugins::component_index<T>()));
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