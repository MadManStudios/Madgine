#pragma once

namespace Engine {
namespace Execution {

    struct StopCallback {

        virtual void stopRequested() = 0;
        void set_done() {
            throw 0;
        }

    protected:
        template <typename>
        friend struct ConnectionStack;
        template <typename>
        friend struct ConnectionQueue;

        std::atomic<StopCallback *> mNext = nullptr;
    };

}
}