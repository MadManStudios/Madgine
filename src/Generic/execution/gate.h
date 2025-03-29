#pragma once

namespace Engine {
namespace Execution {

    struct Gate {

        template <typename F>
        bool pass(F&& f) {
            if (mState.fetch_add(1) == 0) {
                std::forward<F>(f)();
            }
            return mState.fetch_sub(1) == 1;
        }

    private:
        std::atomic<uint8_t> mState = 0;
    };

}
}