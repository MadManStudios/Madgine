#pragma once

#include "Modules/threading/madgineobject.h"

#include "globalapicollector.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "Modules/threading/taskqueue.h"

#include "Madgine/debug/debuggablelifetime.h"

namespace Engine {
namespace App {

    /**
     * @brief The Application manages all GlobalAPIComponents
     * 
     * The Application creates all registered GlobalAPIComponents.
     * It also creates a TaskQueue for game logic tasks.
    */
    struct MADGINE_APP_EXPORT Application : Threading::MadgineObject<Application> {
        using Self = Application;

        Application();
        ~Application();

        template <typename T>
        T &getGlobalAPIComponent()
        {
            return static_cast<T &>(getGlobalAPIComponent(UniqueComponent::component_index<T>()));
        }

        GlobalAPIBase &getGlobalAPIComponent(size_t i);

        Threading::TaskQueue *taskQueue();

        void startLifetime();
        void endLifetime();

        Debug::DebuggableLifetime<> &lifetime();

        static Application &getSingleton();
        static Application *getSingletonPtr();

    protected:
        /**
     * @name MadgineObject interface
    */
        ///@{
        Threading::Task<bool> init();
        Threading::Task<void> finalize();
        friend struct MadgineObject<Application>;
        ///@}

    private:
        Threading::TaskQueue mTaskQueue;

        DEBUGGABLE_LIFETIME(mLifetime);

    public:
        GlobalAPIContainer<std::vector<Placeholder<0>>> mGlobalAPIs;
    };
}
}

REGISTER_TYPE(Engine::App::Application)
