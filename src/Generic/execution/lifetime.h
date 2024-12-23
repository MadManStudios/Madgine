#pragma once

#include "Generic/genericresult.h"

namespace Engine {
namespace Execution {

    struct Lifetime {

        ~Lifetime()
        {
            assert(mCount == 0);
        }

        void start()
        {
            mStopSource = {};
            unsigned int previous = mCount.fetch_add(1);
            assert(previous == 0);
        }

        template <typename Sender>
        void attach(Sender &&sender)
        {
            unsigned int count = mCount;
            if (!mStopSource.stop_requested()) {
                assert(count > 0);
                (new attach_state<Sender> { std::forward<Sender>(sender), *this })->start();
            }
        }

        void end()
        {
            assert(mCount > 0);
            if (mStopSource.request_stop())
                decreaseCount();
        }

        bool running() const
        {
            return mCount > 0;
        }

        using is_sender = void;

        using result_type = GenericResult;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(connect_t, Lifetime &lifetime, Rec &&rec)
        {
            return state<Rec>(std::forward<Rec>(rec), lifetime);
        }

    private:
        void setReceiver(VirtualReceiverBase<GenericResult> *receiver)
        {
            increaseCount();
            assert(!mReceiver);
            mReceiver = receiver;
            decreaseCount();
        }

        void increaseCount()
        {
            ++mCount;
        }

        void decreaseCount()
        {
            if (mCount.fetch_sub(1) == 1) {
                if (mReceiver) {
                    mReceiver->set_value();
                    mReceiver = nullptr;
                }
            }
        }

        template <typename Sender>
        struct attach_state;

        template <typename Sender>
        struct attach_receiver : execution_receiver<> {

            void set_value()
            {
                mState->mLifetime.decreaseCount();
                delete mState;
            }

            template <typename... V>
            [[nodiscard]] std::monostate set_value(V &&...)
            {
                mState->mLifetime.decreaseCount();
                delete mState;
                return {};
            }
            void set_done()
            {
                mState->mLifetime.decreaseCount();
                delete mState;
            }
            template <typename... R>
            [[nodiscard]] std::monostate set_error(R &&...)
            {
                mState->mLifetime.decreaseCount();
                delete mState;
                return {};
            }

            friend auto tag_invoke(get_stop_token_t, attach_receiver<Sender> &rec)
            {
                return rec.mToken;
            }

            attach_state<Sender> *mState;
            std::stop_token mToken;
        };

        template <typename Sender>
        struct attach_state {
            attach_state(Sender &&sender, Lifetime &lifetime)
                : mState(connect(std::forward<Sender>(sender), attach_receiver<Sender> { {}, this, lifetime.mStopSource.get_token() }))
                , mLifetime(lifetime)
            {
            }
            void start()
            {
                mLifetime.increaseCount();
                mState.start();
            }

            connect_result_t<Sender, attach_receiver<Sender>> mState;
            Lifetime &mLifetime;
        };

        template <typename Rec>
        struct state : VirtualReceiverBase<GenericResult> {
            state(Rec &&rec, Lifetime &lifetime)
                : mRec(std::forward<Rec>(rec))
                , mCallback(get_stop_token(mRec), callback { lifetime })
                , mLifetime(lifetime)
            {
            }

            void start()
            {
                mLifetime.setReceiver(this);
            }

            virtual void set_value() override
            {
                mRec.set_value();
            }

            virtual void set_error(GenericResult result) override
            {
                mRec.set_error(result);
            }

            virtual void set_done() override
            {
                mRec.set_done();
            }

            struct callback {
                void operator()()
                {
                    mLifetime.end();
                }

                Lifetime &mLifetime;
            };

            Rec mRec;
            std::stop_callback<callback> mCallback;
            Lifetime &mLifetime;
        };

        std::stop_source mStopSource;
        std::atomic<uint32_t> mCount = 0;
        VirtualReceiverBase<GenericResult> *mReceiver = nullptr;
    };

}
}