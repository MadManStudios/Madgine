#include "../applib.h"

#include "application.h"

#include "Generic/execution/execution.h"

#include "Modules/threading/workgroupstorage.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "globalapibase.h"

METATABLE_BEGIN(Engine::App::Application)
    MEMBER(mGlobalAPIs)
METATABLE_END(Engine::App::Application)

namespace Engine {

namespace App {

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
        mTaskQueue.addSetupSteps(
            [this]() { return callInit(); },
            [this]() { return callFinalize(); });
    }

    /**
     * @brief
     */
    Application::~Application()
    {
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
        mTaskQueue.queue([this]() -> Threading::ImmediateTask<void> {
            co_await mLifetime;
        });

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
