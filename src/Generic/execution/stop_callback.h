#pragma once

#include "../manuallifetime.h"

namespace Engine {
namespace Execution {

    enum ExecutionState {
        FINISH_STARTED = 1 << 0,
        FINISH_ENDED = 1 << 1,
        STOP_STARTED = 1 << 2,
        STOP_FAILED = 1 << 3
    };

    struct cancelled_t {
    };
    constexpr cancelled_t cancelled;

    template <typename stop_cb, typename finally_cb>
    struct stop_callback : finally_cb {

        template <typename... FC>
        stop_callback(FC &&...fc)
            : finally_cb(std::forward<FC>(fc)...)
        {
        }

        template <typename SC>
        void start(std::stop_token st, SC &&sc)
        {
            construct(mCallback, std::move(st), inner_callback { *this, std::forward<SC>(sc) });
        }

        template <typename... Args>
        void finish(Args &&...args)
        {
            uint8_t state = mState.fetch_or(FINISH_STARTED);
            assert(!(state & FINISH_STARTED));
            assert(!(state & FINISH_ENDED));

            if (!(state & STOP_STARTED) || (state & STOP_FAILED))
                destruct(mCallback);

            state = mState.fetch_or(FINISH_ENDED);
            assert(state & FINISH_STARTED);
            assert(!(state & FINISH_ENDED));
            mState = 0;
            if (state & STOP_FAILED)
                finally_cb::operator()(cancelled);
            else if (!(state & STOP_STARTED))
                finally_cb::operator()(std::forward<Args>(args)...);
        }

    protected:
        void stop(stop_cb &cb)
        {
            uint8_t state = mState.fetch_or(STOP_STARTED);
            assert(!(state & STOP_STARTED));
            assert(!(state & STOP_FAILED));

            bool stopped = cb();
            assert(!stopped || !(state & FINISH_STARTED));
            if (stopped) {
                destruct(mCallback);
                mState = 0;
                finally_cb::operator()(cancelled);
            } else {
                state = mState.fetch_or(STOP_FAILED);
                if (state & FINISH_ENDED) {
                    destruct(mCallback);
                    mState = 0;
                    finally_cb::operator()(cancelled);
                }
            }
        }

    private:
        struct inner_callback : stop_cb {

            template <typename C>
            inner_callback(stop_callback &callback, C &&c)
                : stop_cb(std::forward<C>(c))
                , mCallback(callback)
            {
            }

            void operator()()
            {
                mCallback.stop(*this);
            }

            stop_callback &mCallback;
        };

        std::atomic<uint8_t> mState;
        ManualLifetime<std::stop_callback<inner_callback>> mCallback;
    };
}
}