#pragma once

#include "Generic/execution/concepts.h"
#include "Generic/type_pack.h"
#include "Generic/genericresult.h"
#include "Generic/execution/connection.h"
#include "Generic/execution/container/queue.h"

namespace Engine {
namespace Threading {

    struct MODULES_EXPORT DataMutex {

        struct LockState {
            LockState(DataMutex *mutex, AccessMode mode)
                : mMutex(mutex)
                , mMode(mode)
            {
            }

            virtual void onLockAcquired() = 0;
            void cancel() {
                throw 0;
            }

            DataMutex *mMutex = nullptr;
            AccessMode mMode;

            std::atomic<LockState *> mNext = nullptr;
        };

        struct MODULES_EXPORT Lock : private LockState {
            Lock(DataMutex &mutex, AccessMode mode);
            ~Lock();

            bool isHeld() const;

        protected:
            virtual void onLockAcquired() override;

        private:
            std::mutex mCvMutex;
            std::condition_variable mCv;
            bool mWasTriggered = false;
        };

        template <typename Rec, typename F>
        struct state : LockState, Execution::base_state<Rec> {

            state(DataMutex *mutex, AccessMode mode, Rec &&rec, F &&f)
                : LockState(mutex, mode)
                , Execution::base_state<Rec>(std::forward<Rec>(rec))
                , mF(std::forward<F>(f))
            {
            }

            void start()
            {
                if (mMutex->lockImpl(this)) {
                    onLockAcquired();
                }
            }

            virtual void onLockAcquired() override
            {
                if constexpr (std::same_as<std::invoke_result_t<F>, void>) {
                    mF();
                    mMutex->unlockImpl(this);
                    mRec.set_value();
                } else {
                    auto &&result = mF();
                    mMutex->unlockImpl(this);
                    mRec.set_value(std::forward<decltype(result)>(result));
                }
            }

            F mF;
        };

        template <typename F>
        struct sender {

            using is_sender = void;

            template <typename T>
            static auto return_types_helper()
            {
                if constexpr (std::same_as<T, void>) {
                    return type_pack<> {};
                } else if constexpr (InstanceOf<T, std::tuple>) {
                    return to_type_pack<T> {};
                } else {
                    return type_pack<T> {};
                }
            }

            template <template <typename...> typename Tuple>
            using value_types = typename decltype(return_types_helper<std::invoke_result_t<F>>())::template instantiate<Tuple>;
            using result_type = GenericResult;

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, F> { sender.mMutex, sender.mMode, std::forward<Rec>(rec), std::forward<F>(sender.mF) };
            }

            DataMutex *mMutex;
            AccessMode mMode;
            F mF;
        };

        DataMutex(std::string_view name);
        ~DataMutex();

        const std::string &name() const;

        Lock lock(AccessMode mode);

        template <typename F>
        auto locked(AccessMode mode, F &&f)
        {
            return sender<F> { this, mode, std::forward<F>(f) };
        }

    protected:
        bool lockImpl(LockState *state);
        void unlockImpl(LockState *state);

        bool lockRead(LockState *state);
        bool lockWrite(LockState *state);
        void unlockRead(LockState *state);
        void unlockWrite(LockState *state);

        bool unrollRead();
        bool unrollWrite();

    private:
        std::shared_mutex mMutex;
        std::string mName;

        Execution::ConnectionQueue<LockState> mReadQueue;
        Execution::ConnectionQueue<LockState> mWriteQueue;        
    };

}
}