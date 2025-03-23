#include "../applib.h"
#include "application.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Modules/threading/workgroupstorage.h"

#include "globalapibase.h"

#include "Modules/threading/awaitables/awaitablesender.h"

METATABLE_BEGIN(Engine::App::Application)
MEMBER(mGlobalAPIs)
METATABLE_END(Engine::App::Application)

namespace Engine {

namespace App {

    static Threading::WorkgroupLocal<Application *> sApp;

    /**
     * @brief Creates an Application and sets up its TaskQueue
     * 
     * Instantiates all WindowAPIComponents. Initialization/Deinitialization-tasks
     * of the MadgineObject are registered as setup steps in the TaskQueue.
    */
    Application::Application()
        : mTaskQueue("Application")
        , mGlobalAPIs(*this)
    {
        assert(!sApp);
        sApp = this;

        mTaskQueue.addSetupSteps(
            [this]() { return callInit(); },
            [this]() { return callFinalize(); });
    }

    /**
     * @brief 
    */
    Application::~Application()
    {
        assert(sApp == this);
        sApp = nullptr;
    }

    /**
     * @brief 
     * @return 
    */
    Threading::Task<bool> Application::init()
    {
        for (const std::unique_ptr<GlobalAPIBase> &api : mGlobalAPIs) {
            if (!co_await api->callInit())
                co_return false;
        }        

        startLifetime();

        co_return true;
    }

    /**
     * @brief 
     * @return 
    */
    Threading::Task<void> Application::finalize()
    {
        endLifetime();
        co_await mLifetime.finished();

        for (const std::unique_ptr<GlobalAPIBase> &api : mGlobalAPIs) {
            co_await api->callFinalize();
        }
    }

    /**
     * @brief 
     * @param i 
     * @return 
    */
    GlobalAPIBase &Application::getGlobalAPIComponent(size_t i)
    {
        return mGlobalAPIs.get(i);
    }

    /**
     * @brief 
     * @return 
    */
    Application &Application::getSingleton()
    {
        return *sApp;
    }

    /**
     * @brief 
     * @return 
    */
    Application *Application::getSingletonPtr()
    {
        return sApp;
    }

    /**
     * @brief 
     * @return 
    */
    Threading::TaskQueue *Application::taskQueue()
    {
        return &mTaskQueue;
    }

    Debug::DebuggableLifetime<> &Application::lifetime()
    {
        return mLifetime;
    }

    void Application::startLifetime()
    {
        Execution::detach(mLifetime);

        for (const std::unique_ptr<GlobalAPIBase> &api : mGlobalAPIs) {
            api->startLifetime();
        }
    }

    void Application::endLifetime()
    {
        mLifetime.end();
    }
}

}
