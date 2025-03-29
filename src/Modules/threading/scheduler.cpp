#include "../moduleslib.h"

#include "scheduler.h"

#include "workgroup.h"

#include "taskqueue.h"

#include "Interfaces/threading/threadapi.h"

namespace Engine {
namespace Threading {

    Scheduler::Scheduler()
        : mWorkgroup(WorkGroup::self())
    {
    }

    int Scheduler::go()
    {
#if !EMSCRIPTEN
        Threading::TaskQueue *main_queue = nullptr;

        for (Threading::TaskQueue *queue : mWorkgroup.taskQueues()) {
            if (queue->wantsMainThread()) {
                if (main_queue)
                    throw 0;
                main_queue = queue;
            }
        }

        if (!main_queue)
            main_queue = mWorkgroup.taskQueues().front();

        for (Threading::TaskQueue *queue : mWorkgroup.taskQueues()) {
            if (queue != main_queue)
                mWorkgroup.createThread(&Scheduler::schedulerLoop, this, queue);
        }

        setupThreadInfo(main_queue, " (Main)");

        while (mWorkgroup.state() != WorkGroupState::DONE || !mWorkgroup.singleThreaded()){
            main_queue->update();
            mWorkgroup.update();
        }

        for (Threading::TaskQueue *queue : mWorkgroup.taskQueues()) {
            assert(queue->idle());
        }

#endif
        return 0;
    }

#if !EMSCRIPTEN
    void Scheduler::schedulerLoop(Threading::TaskQueue *queue)
    {
        setupThreadInfo(queue);
        while (mWorkgroup.state() != WorkGroupState::DONE) {
            queue->update();
        }
        assert(queue->idle());
    }
#endif

    void Scheduler::setupThreadInfo(Threading::TaskQueue *queue, std::string tags)
    {
#if MODULES_ENABLE_TASK_TRACKING
        queue->mTracker.mThread = std::this_thread::get_id();
#endif
        setCurrentThreadName(mWorkgroup.name() + "_" + queue->name() + tags);
    }

}
}