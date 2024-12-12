#pragma once

namespace Engine {
namespace Threading {

    ENUM(WorkGroupState,
        INITIALIZING,
        RUNNING,
        STOPPING,
        FINALIZING,
        DONE);

    struct MODULES_EXPORT WorkGroup {
        WorkGroup(std::string_view name = {});
        WorkGroup(const WorkGroup &) = delete;
        ~WorkGroup();

        void operator=(const WorkGroup &) = delete;

        WorkGroupState state() const;

        void update();

        void stop();

#if ENABLE_THREADING
        template <typename F, typename... Args>
        void createThread(F &&main, Args &&...args)
        {
            std::unique_lock lock { mThreadsMutex };
            mSubThreads.emplace_back(
                std::async(std::launch::async, &WorkGroup::threadMain<F, std::decay_t<Args>...>, this, std::forward<F>(main), std::forward<Args>(args)...));
        }

        template <typename F, typename... Args>
        auto spawnTaskThread(F &&task, Args &&...args)
        {
            return std::async(std::launch::async, &WorkGroup::taskMain<F, std::decay_t<Args>...>, this, std::forward<F>(task), std::forward<Args>(args)...);
        }

        bool singleThreaded();

        bool contains(std::thread::id id) const;
#endif

        void addThreadInitializer(std::function<void()> &&task);
        static void addStaticThreadGuards(std::function<void()> &&init, std::function<void()> &&finalize = {});

        const std::string &name() const;

        static WorkGroup &self();
        static bool isInitialized();

        void addTaskQueue(TaskQueue *taskQueue);
        void removeTaskQueue(TaskQueue *taskQueue);
        const std::vector<TaskQueue *> &taskQueues() const;

    private:
#if ENABLE_THREADING
        void initThread();
        void finalizeThread();

        struct ThreadGuard {
            ThreadGuard(WorkGroup &group)
                : mGroup(group)
            {
                mGroup.initThread();
            }

            ~ThreadGuard()
            {
                mGroup.finalizeThread();
            }

        private:
            WorkGroup &mGroup;
        };

        template <typename F, typename... Args>
        int threadMain(F &&main, Args &&...args)
        {
            ThreadGuard guard(*this);
            try {

                int result = TupleUnpacker::invokeDefaultResult(0, std::forward<F>(main), std::forward<Args>(args)...);
                return result;
            } catch (std::exception &e) {
                LOG_ERROR("Uncaught Exception in Workgroup-Thread!");
                LOG_ERROR(e.what());
                throw;
            } catch (...) {
                std::terminate();
            }
        }

        template <typename F, typename... Args>
        auto taskMain(F &&main, Args &&...args)
        {
            ThreadGuard guard(*this);
            try {
                return std::forward<F>(main)(std::forward<Args>(args)...);
            } catch (std::exception &e) {
                LOG_ERROR("Uncaught Exception in Workgroup-Task-Thread!");
                LOG_ERROR(e.what());
                throw;
            }
        }
#endif

        void setState(WorkGroupState state);

    private:
        friend struct WorkGroupStorage;

        size_t mInstanceCounter = 0;
        std::string mName;

#if ENABLE_THREADING
        mutable std::mutex mThreadsMutex;

        std::vector<std::thread::id> mThreads;

        std::vector<std::future<int>> mSubThreads;
#endif
        std::vector<std::function<void()>> mThreadInitializers;

        std::vector<TaskQueue *> mTaskQueues;

        std::atomic<WorkGroupState::BaseType> mState;
    };
}
}
