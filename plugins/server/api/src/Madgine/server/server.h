#pragma once

#include "Modules/threading/taskqueue.h"
#include "serverinstance.h"

#include "Modules/threading/madgineobject.h"

#include "serverapicollector.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

namespace Engine {
namespace App {
    struct Application;
}
namespace Window {
    struct MainWindow;
}
namespace Server {
    struct MADGINE_SERVER_EXPORT Server : Threading::MadgineObject<Server> {
        Server(std::function<int(Closure<void(Engine::App::Application &, Engine::Window::MainWindow &)>)> entrypoint);
        ~Server();

        void spawnInstance(std::string_view name, Closure<void(Engine::App::Application &, Engine::Window::MainWindow &)>);

        template <typename T>
        T &getServerAPIComponent()
        {
            return static_cast<T &>(getServerAPIComponent(UniqueComponent::component_index<T>()));
        }

        ServerAPIBase &getServerAPIComponent(size_t i);

        Threading::TaskQueue *taskQueue();

    protected:
        Threading::Task<bool> init();
        Threading::Task<void> finalize();
        friend struct MadgineObject<Server>;

    private:
        std::list<ServerInstance> mInstances;

        Threading::TaskQueue mTaskQueue;

        std::function<int(Closure<void(Engine::App::Application &, Engine::Window::MainWindow &)>)> mEntrypoint;

    public:
        ServerAPIContainer<std::vector<Placeholder<0>>> mServerAPIs;
    };

}
}

REGISTER_TYPE(Engine::Server::Server)
