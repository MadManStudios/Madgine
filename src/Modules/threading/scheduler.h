#pragma once

namespace Engine {
namespace Threading {

    struct MODULES_EXPORT Scheduler {
        Scheduler();

        int go();

    private:
#if !EMSCRIPTEN
        void schedulerLoop(Threading::TaskQueue *queue);
#endif

        void setupThreadInfo(Threading::TaskQueue *queue, std::string tags = "");

    private:
        WorkGroup &mWorkgroup;
    };

}
}