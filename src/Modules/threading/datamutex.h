#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/execution/connection.h"
#include "Generic/execution/container/queue.h"

namespace Engine {
namespace Threading {

    struct MODULES_EXPORT DataMutex {

        struct LockState {
            LockState(DataMutex &mutex, AccessMode mode)
                : mMutex(mutex)
                , mMode(mode)
            {
            }

            virtual bool onLockAcquired() = 0;
            virtual void complete() = 0;
            void cancel()
            {
                throw 0;
            }

            DataMutex &mMutex;
            AccessMode mMode;

            std::atomic<LockState *> mNext = nullptr;
        };

        struct MODULES_EXPORT Lock : private LockState {
            Lock(DataMutex &mutex, AccessMode mode);
            ~Lock();

        protected:
            bool onLockAcquired() override;
            void complete() override;

        private:
            std::mutex mCvMutex;
            std::condition_variable mCv;
            bool mWasTriggered = false;
        };

        template <typename Rec, typename F>
        struct state : LockState, Execution::base_state<Rec> {

            state(DataMutex &mutex, AccessMode mode, Rec &&rec, F &&f)
                : LockState(mutex, mode)
                , Execution::base_state<Rec>(std::forward<Rec>(rec))
                , mF(std::forward<F>(f))
            {
            }

            void start()
            {
                if (mMutex.lockImpl(this)) {
                    mMutex.run(this);
                }
            }

            void stop()
            {
                if (mMutex.cancel(this))
                    this->set_done();
            }

            bool onLockAcquired() override
            {
                if constexpr (std::same_as<std::invoke_result_t<F>, void>) {
                    mF();
                } else {
                    construct(mResult, mF());
                }
                return true;
            }

            void complete() override
            {
                if constexpr (std::same_as<std::invoke_result_t<F>, void>) {
                    this->mRec.set_value();
                } else {
                    std::invoke_result_t<F> result = std::forward<std::invoke_result_t<F>>(*mResult);
                    destruct(mResult);
                    this->mRec.set_value(std::forward<std::invoke_result_t<F>>(result));
                }
            }

            friend auto tag_invoke(Execution::visit_state_t, state *state, auto &&visitor)
            {
                visitor(Execution::State::Text { typeid(F).name() });
            }

            F mF;
            ManualLifetime<forward_ref_t<patch_void_t<std::invoke_result_t<F>>>> mResult;
        };

        template <typename F>
        struct sender {

            using is_sender = void;

            template <template <typename...> typename Tuple>
            using value_types = typename to_type_pack<std::invoke_result_t<F>>::template instantiate<Tuple>;
            using result_type = void;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, F> { sender.mMutex, sender.mMode, std::forward<Rec>(rec), std::forward<F>(sender.mF) };
            }

            DataMutex &mMutex;
            AccessMode mMode;
            F mF;
        };

        DataMutex(std::string_view name, TaskQueue &taskQueue);
        ~DataMutex();

        const std::string &name() const;

        Lock lock(AccessMode mode);

        template <typename F>
        auto locked(AccessMode mode, F &&f)
        {
            return sender<F> { *this, mode, std::forward<F>(f) };
        }

    protected:
        bool lockImpl(LockState *state);
        LockState *unlockImpl(LockState *state);

        bool lockRead(LockState *state);
        bool lockWrite(LockState *state);
        LockState *unlockRead();
        LockState *unlockWrite();

        void run(LockState *state, bool onThread);
        bool cancel(LockState *state);

    private:
        std::shared_mutex mMutex;
        std::string mName;

        TaskQueue &mTaskQueue;

        Execution::ConnectionQueue<LockState> mReadQueue;
        Execution::ConnectionQueue<LockState> mWriteQueue;
    };

}
}