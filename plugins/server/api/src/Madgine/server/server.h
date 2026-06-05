#pragma once

#include "Modules/threading/madgineobject.h"
#include "Modules/threading/taskqueue.h"
#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "serverapicollector.h"
#include "serverinstance.h"

namespace Engine {
namespace Core {
    struct Application;
    struct MainWindow;

    struct MADGINE_SERVER_EXPORT Server : Threading::MadgineObject<Server> {
        Server(std::function<int(Closure<void(Application &, MainWindow &)>)> entrypoint);
        ~Server();

        void spawnInstance(std::string_view name, Closure<void(Application &, MainWindow &)>);

        template <typename T>
        T &getServerAPIComponent()
        {
            return static_cast<T &>(getServerAPIComponent(Plugins::component_index<T>()));
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

        std::function<int(Closure<void(Core::Application &, Core::MainWindow &)>)> mEntrypoint;

    public:
        ServerAPIContainer<std::vector<Placeholder<0>>> mServerAPIs;
    };

}
}
