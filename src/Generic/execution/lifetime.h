#pragma once

#include "Generic/genericresult.h"

namespace Engine {
namespace Execution {

    template <auto... cpos>
    struct Lifetime {

        Lifetime() {
            mStopSource.request_stop();
        }

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

        struct ended_sender;

        ended_sender end()
        {
            assert(mCount > 0);
            if (mStopSource.request_stop())
                decreaseCount();
            return { *this };
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

        struct ended_sender {
            using is_sender = void;

            using result_type = GenericResult;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, ended_sender &&sender, Rec &&rec)
            {
                return ended_state<Rec>(std::forward<Rec>(rec), sender.mLifetime);
            }

            Lifetime &mLifetime;
        };

    private:
        using LifetimeReceiver = VirtualReceiverBaseEx<type_pack<GenericResult>, type_pack<>, cpos...>;

        void setReceiver(LifetimeReceiver *receiver)
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

            friend std::stop_token tag_invoke(get_stop_token_t, attach_receiver<Sender> &rec)
            {
                return rec.mState->mLifetime.mStopSource.get_token();
            }

            template <typename CPO, typename... Args>
            requires(is_tag_invocable_v<CPO, LifetimeReceiver &, Args...>) friend auto tag_invoke(CPO f, attach_receiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, LifetimeReceiver &, Args...>)
                -> tag_invoke_result_t<CPO, LifetimeReceiver &, Args...>
            {
                return tag_invoke(f, *rec.mState->mLifetime.mReceiver, std::forward<Args>(args)...);
            }

            attach_state<Sender> *mState;
        };

        template <typename Sender>
        struct attach_state {
            attach_state(Sender &&sender, Lifetime &lifetime)
                : mLifetime(lifetime)
                , mState(connect(std::forward<Sender>(sender), attach_receiver<Sender> { {}, this }))
            {
            }
            void start()
            {
                mLifetime.increaseCount();
                mState.start();
            }

            Lifetime &mLifetime;
            connect_result_t<Sender, attach_receiver<Sender>> mState;
        };

        template <typename Rec>
        struct ended_state : VirtualState<Rec, LifetimeReceiver> {
            ended_state(Rec &&rec, Lifetime &lifetime)
                : VirtualState<Rec, LifetimeReceiver>(std::forward<Rec>(rec))
                , mLifetime(lifetime)
            {
            }

            void start()
            {
                mLifetime.setReceiver(this);
            }

            Lifetime &mLifetime;
        };

        template <typename Rec>
        struct state : ended_state<Rec> {
            state(Rec &&rec, Lifetime &lifetime)
                : ended_state<Rec>(std::forward<Rec>(rec), lifetime)
                , mCallback(get_stop_token(mRec), callback { lifetime })
            {
            }

            void start()
            {
                mLifetime.start();
                ended_state::start();
            }

            struct callback {
                void operator()()
                {
                    mLifetime.end();
                }

                Lifetime &mLifetime;
            };

            std::stop_callback<callback> mCallback;
        };

        std::stop_source mStopSource;
        std::atomic<uint32_t> mCount = 0;
        LifetimeReceiver *mReceiver = nullptr;
    };

}
}