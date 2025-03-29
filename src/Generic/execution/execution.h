#pragma once

#include "../makeowning.h"
#include "../withresult.h"
#include "concepts.h"
#include "storage.h"

namespace Engine {
namespace Execution {

    struct detach_t {

        template <typename Sender>
        struct state;

        template <typename Sender>
        struct receiver : execution_receiver<> {

            template <typename... V>
            void set_value(V &&...)
            {
                delete mState;
            }
            void set_done()
            {
                delete mState;
            }
            template <typename... R>
            void set_error(R &&...)
            {
                delete mState;
            }

            state<Sender> *mState;
        };

        template <typename Sender>
        struct state {
            state(Sender &&sender)
                : mState(connect(std::forward<Sender>(sender), receiver<Sender> { {}, this }))
            {
            }
            void start()
            {
                mState.start();
            }

            connect_result_t<Sender, receiver<Sender>> mState;
        };

        template <typename Sender>
        void operator()(Sender &&sender) const
        {
            (new state<Sender> { std::forward<Sender>(sender) })->start();
        }
    };

    inline constexpr detach_t detach;

    struct sync_expect_t {

        template <typename Sender>
        struct state;

        template <typename Sender>
        struct receiver : execution_receiver<> {

            template <typename... V>
            void set_value(V &&... v)
            {
                mState->mResult.set_value(std::forward<V>(v)...);
                mState->mFinished = true;
            }
            void set_done()
            {
                mState->mResult.set_done();
                mState->mFinished = true;
            }
            template <typename... R>
            void set_error(R &&...r)
            {
                mState->mResult.set_error(std::forward<R>(r)...);
                mState->mFinished = true;
            }

            state<Sender> *mState;
        };

        template <typename Sender>
        struct state {
            state(Sender &&sender)
                : mState(connect(std::forward<Sender>(sender), receiver<Sender> { {}, this }))
            {
            }
            void start()
            {
                mState.start();

            }

            ResultStorage<Sender> mResult;
            bool mFinished;
            connect_result_t<Sender, receiver<Sender>> mState;
        };

        template <typename Sender>
        auto operator()(Sender &&sender) const
        {
            state<Sender> state { std::forward<Sender>(sender) };
            state.start();
            assert(state.mFinished);
            return std::move(state.mResult);
        }
    };

    inline constexpr sync_expect_t sync_expect;

    struct sync_wait_t {

        template <typename Sender>
        struct state;

        template <typename Sender>
        struct receiver : execution_receiver<> {

            template <typename... V>
            void set_value(V &&...v)
            {
                mState->mResult.set_value(std::forward<V>(v)...);
                mState->mFinished.test_and_set();
            }
            void set_done()
            {
                mState->mResult.set_done();
                mState->mFinished.test_and_set();
            }
            template <typename... R>
            void set_error(R&&...r)
            {
                mState->mResult.set_error(std::forward<R>(r)...);
                mState->mFinished.test_and_set();
            }

            state<Sender> *mState;
        };

        template <typename Sender>
        struct state {
            state(Sender &&sender)
                : mState(connect(std::forward<Sender>(sender), receiver<Sender> { {}, this }))
            {
            }
            void start()
            {
                mState.start();
            }

            ResultStorage<Sender> mResult;
            std::atomic_flag mFinished;
            connect_result_t<Sender, receiver<Sender>> mState;
        };

        template <typename Sender>
        auto operator()(Sender &&sender) const
        {
            state<Sender> state { std::forward<Sender>(sender) };
            state.start();
            state.mFinished.wait(false);
            return std::move(state.mResult);
        }
    };

    inline constexpr sync_wait_t sync_wait;

}
}
