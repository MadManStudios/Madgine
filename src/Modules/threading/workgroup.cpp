#include "../moduleslib.h"

#if ENABLE_THREADING
#    include "Interfaces/threading/threadapi.h"
#    include "threadlocal.h"
#    include "threadstorage.h"
#endif

#include "workgroup.h"
#include "workgroupstorage.h"

#include "taskqueue.h"

namespace Engine {
namespace Threading {

#if ENABLE_THREADING
    static THREADLOCAL(WorkGroup *) sSelf = nullptr;
#else
    static WorkGroup *sSelf = nullptr;
#endif

    static std::atomic<size_t> sWorkgroupInstanceCounter = 0;

    static std::vector<std::function<void()>> &sThreadInitializers()
    {
        static std::vector<std::function<void()>> dummy;
        return dummy;
    }

    static std::vector<std::function<void()>> &sThreadFinalizers()
    {
        static std::vector<std::function<void()>> dummy;
        return dummy;
    }

    WorkGroup::WorkGroup(std::string_view name)
        : mInstanceCounter(sWorkgroupInstanceCounter++)
        , mName(name.empty() ? "Workgroup_" + std::to_string(mInstanceCounter) : name)
        , mState(WorkGroupState::INITIALIZING)
    {
#if ENABLE_THREADING
        mThreads.push_back(std::this_thread::get_id());

        ThreadStorage::init(true);
        ThreadStorage::init(false);

        setCurrentThreadName(mName + "_Main");
#endif

        assert(!sSelf);
        sSelf = this;

        WorkGroupStorage::init(true);
        WorkGroupStorage::init(false);

        for (const std::function<void()> &task : sThreadInitializers()) {
            task();
        }

        //Is that useful at all?
        for (const std::function<void()> &task : mThreadInitializers) {
            task();
        }
    }

    WorkGroup::~WorkGroup()
    {
        for (const std::function<void()> &task : sThreadFinalizers()) {
            task();
        }

        WorkGroupStorage::finalize(false);
        WorkGroupStorage::finalize(true);

        assert(sSelf == this);
        sSelf = nullptr;

#if ENABLE_THREADING
        ThreadStorage::finalize(false);
        ThreadStorage::finalize(true);

        assert(singleThreaded());
#endif
    }

    WorkGroupState WorkGroup::state() const
    {
        return { mState };
    }

    void WorkGroup::stop()
    {
        if (mState == WorkGroupState::INITIALIZING || mState == WorkGroupState::RUNNING)
            setState(WorkGroupState::STOPPING);
    }

    void WorkGroup::addThreadInitializer(std::function<void()> &&task)
    {
#if ENABLE_THREADING
        {
            std::unique_lock lock { mThreadsMutex };
            assert(mSubThreads.empty());
        }
#endif
        mThreadInitializers.emplace_back(std::move(task));
    }

    void WorkGroup::addStaticThreadGuards(std::function<void()> &&init, std::function<void()> &&finalize)
    {
        sThreadInitializers().emplace_back(std::move(init));
        if (finalize)
            sThreadFinalizers().emplace_back(std::move(finalize));
    }

    const std::string &WorkGroup::name() const
    {
        return mName;
    }

    void WorkGroup::update()
    {
#if ENABLE_THREADING
        {
            std::unique_lock lock { mThreadsMutex };
            std::erase_if(mSubThreads,
                [](std::future<int> &f) {
                    bool result = f.wait_for(0ms) != std::future_status::timeout;
                    if (result)
                        f.get();
                    return result;
                });
        }
#endif

        if (mState != WorkGroupState::RUNNING) {
            for (TaskQueue *queue : mTaskQueues)
                if (!queue->idle())
                    return;

            switch (mState) {
            case WorkGroupState::INITIALIZING:
                setState(WorkGroupState::RUNNING);
                break;
            case WorkGroupState::STOPPING:
                setState(WorkGroupState::FINALIZING);
                break;
            case WorkGroupState::FINALIZING:
                setState(WorkGroupState::DONE);
                break;
            }
        }
    }

#if ENABLE_THREADING
    bool WorkGroup::singleThreaded()
    {
        return mSubThreads.empty();
    }

    bool WorkGroup::contains(std::thread::id id) const
    {
        std::unique_lock lock { mThreadsMutex };
        return std::ranges::find(mThreads, id) != mThreads.end();
    }

    void WorkGroup::initThread()
    {
        {
            std::unique_lock lock { mThreadsMutex };
            mThreads.push_back(std::this_thread::get_id());
        }

        ThreadStorage::init(true);
        ThreadStorage::init(false);

        assert(!sSelf);
        sSelf = this;

        for (const std::function<void()> &task : sThreadInitializers()) {
            task();
        }

        for (const std::function<void()> &task : mThreadInitializers) {
            task();
        }
    }

    void WorkGroup::finalizeThread()
    {
        assert(sSelf == this);
        sSelf = nullptr;

        for (const std::function<void()> &task : sThreadFinalizers()) {
            task();
        }

        ThreadStorage::finalize(false);
        ThreadStorage::finalize(true);
    }
#endif

    void WorkGroup::setState(WorkGroupState state)
    {
        LOG_DEBUG("Changing Workgroup state to " << state);
        mState = state;
#if !EMSCRIPTEN
        for (TaskQueue *queue : mTaskQueues)
            queue->notify();
#endif
    }

    WorkGroup &WorkGroup::self()
    {
        assert(isInitialized());
        return *sSelf;
    }

    bool WorkGroup::isInitialized()
    {
#if ENABLE_THREADING && USE_CUSTOM_THREADLOCAL
        assert(sSelf.valid());
#endif
        return sSelf != nullptr;
    }

    void WorkGroup::addTaskQueue(TaskQueue *taskQueue)
    {
        mTaskQueues.push_back(taskQueue);
    }

    void WorkGroup::removeTaskQueue(TaskQueue *taskQueue)
    {
        std::erase(mTaskQueues, taskQueue);
    }

    const std::vector<TaskQueue *> &WorkGroup::taskQueues() const
    {
        return mTaskQueues;
    }

}
}