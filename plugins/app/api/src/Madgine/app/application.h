#pragma once

#include "Modules/threading/customclock.h"
#include "Modules/threading/madgineobject.h"
#include "Modules/threading/taskqueue.h"
#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "Madgine/debug/debuggablelifetime.h"

#include "globalapicollector.h"

namespace Engine {
namespace Core {

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
            return static_cast<T &>(getGlobalAPIComponent(Plugins::component_index<T>()));
        }

        GlobalAPIBase &getGlobalAPIComponent(size_t i);

        Threading::TaskQueue &taskQueue();

        void startLifetime();
        void endLifetime();

        Debug::DebuggableLifetime<> &lifetime();

        void pause();
        bool unpause();
        bool isPaused() const;
        const Threading::CustomClock &clock() const;

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

        struct Clock : Threading::CustomClock {
            virtual std::chrono::steady_clock::time_point get(std::chrono::steady_clock::time_point timepoint) const override;
            virtual std::chrono::steady_clock::time_point revert(std::chrono::steady_clock::time_point timepoint) const override;

            std::chrono::steady_clock::duration mPauseAcc = std::chrono::steady_clock::duration::zero();
            std::chrono::steady_clock::time_point mPauseStart;
            std::atomic<size_t> mPauseStack = 0;
        } mClock;

    public:
        GlobalAPIContainer<std::vector<Placeholder<0>>> mGlobalAPIs;
    };
}
}
