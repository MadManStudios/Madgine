#include "../applib.h"

#include "application.h"

#include "Generic/execution/algorithm.h"
#include "Generic/execution/execution.h"

#include "Modules/threading/workgroupstorage.h"
#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "Meta/reflect/metatable_impl.h"

#include "globalapibase.h"

METATABLE_BEGIN(Engine::Core::Application)
    MEMBER(mGlobalAPIs)
METATABLE_END(Engine::Core::Application)

namespace Engine {
namespace Core {

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
        pause();

        addSetupSteps(mTaskQueue);
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

        mTaskQueue.queue([this]() {
            startLifetime();
        });

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
    Threading::TaskQueue &Application::taskQueue()
    {
        return mTaskQueue;
    }

    Debug::DebuggableLifetime<> &Application::lifetime()
    {
        return mLifetime;
    }

    void Application::startLifetime()
    {
        mTaskQueue.queue([this]() -> Threading::ImmediateTask<void> {
            co_await (mLifetime | Execution::after([this]() { unpause(); }) | Execution::finally([this]() { pause(); }));
        });

        for (const std::unique_ptr<GlobalAPIBase> &api : mGlobalAPIs) {
            api->startLifetime();
        }
    }

    void Application::endLifetime()
    {
        mLifetime.end();
    }

    void Application::pause()
    {
        if (mClock.mPauseStack++ == 0) {
            mClock.mPauseStart = std::chrono::steady_clock::now();
        }
    }

    bool Application::unpause()
    {
        assert(mClock.mPauseStack > 0);
        if (--mClock.mPauseStack == 0) {
            mClock.mPauseAcc += std::chrono::steady_clock::now() - mClock.mPauseStart;
            return true;
        }
        return false;
    }

    bool Application::isPaused() const
    {
        return mClock.mPauseStack > 0;
    }

    const Threading::CustomClock &Application::clock() const
    {
        return mClock;
    }

    std::chrono::steady_clock::time_point Application::Clock::get(std::chrono::steady_clock::time_point timepoint) const
    {
        return (mPauseStack > 0 ? mPauseStart : timepoint) - mPauseAcc;
    }

    std::chrono::steady_clock::time_point Application::Clock::revert(std::chrono::steady_clock::time_point timepoint) const
    {
        return timepoint + mPauseAcc + (mPauseStack > 0 ? std::chrono::steady_clock::now() - mPauseStart : 0s);
    }

}
}
