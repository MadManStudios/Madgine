#pragma once

#include "binding.h"
#include "concepts.h"
#include "stop_callback.h"
#include "stop_source.h"
#include "stoppable.h"
#include "virtualstate.h"

namespace Engine {
namespace Execution {

    template <auto... cpos>
    struct Lifetime {
    private:
        struct ControlBlock;

        struct state;

    public:

        struct ControlPtr {
            ControlPtr() = default;
            ControlPtr(ControlBlock &block)
                : mBlock(&block)
            {
                mBlock->increaseWeakCount();
            }
            ~ControlPtr()
            {
                if (mBlock)
                    mBlock->decreaseWeakCount();
            }

            ControlPtr(const ControlPtr &other)
                : mBlock(other.mBlock)
            {
                if (mBlock)
                    mBlock->increaseWeakCount();
            }

            ControlPtr(ControlPtr &&other) noexcept
                : mBlock(std::exchange(other.mBlock, nullptr))
            {
            }

            ControlPtr &operator=(const ControlPtr &other)
            {
                if (this != &other) {
                    if (mBlock) {
                        mBlock->decreaseWeakCount();
                    }
                    mBlock = other.mBlock;
                    if (mBlock) {
                        mBlock->increaseWeakCount();
                    }
                }
                return *this;
            }

            ControlPtr &operator=(ControlPtr &&other) noexcept
            {
                if (this != &other) {
                    if (mBlock) {
                        mBlock->decreaseWeakCount();
                    }
                    mBlock = std::exchange(other.mBlock, nullptr);
                }
                return *this;
            }

            bool alive() const
            {
                return mBlock && mBlock->running();
            }

            explicit operator bool() const
            {
                return mBlock;
            }

            auto operator<=>(const ControlPtr &other) const = default;

            ControlBlock *mBlock = nullptr;
        };
                
        template <typename Rec>
        using virtual_state = VirtualState<state, Rec>;

        Lifetime()
        {
        }

        ~Lifetime()
        {
            assert(!mPtr.alive());
        }

        template <AnySender Sender>
        void attach(Sender &&sender)
        {
            if (mPtr.alive()) {
                (new attach_state<stoppable_t::sender<Sender>> { std::forward<Sender>(sender) | stoppable, mPtr })->start();
            }
        }

        bool end()
        {
            return mPtr.mBlock->request_stop();
        }

        bool running() const
        {
            return mPtr.alive();
        }

        template <typename F>
        struct sender : Execution::base_sender {

            using result_type = void;
            template <template <typename...> typename Tuple>
            using value_types = Tuple<>;

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return virtual_state<Rec>(std::forward<Rec>(rec), sender.mLifetime, std::forward<F>(sender.mCallback));
            }

            Lifetime &mLifetime;
            F mCallback;
        };

        template <typename F>
        sender<F> tracked(F &&callback)
        {
            return sender<F> { {}, *this, std::forward<F>(callback) };
        }

        template <AnyBinding Binding>
        struct BindingPoint {

            using type = typename Binding::type;

            BindingPoint() = default;
            BindingPoint(ControlPtr ptr, Binding &&binding)
                : mPtr(std::move(ptr))
                , mBinding(std::forward<Binding>(binding))
            {
            }

            friend bool tag_invoke(Execution::access_binding_t, const BindingPoint<Binding> &binding, auto &&callback)
            {
                bool result = false;
                auto *block = binding.mPtr.mBlock;
                if (block && block->increaseStrongCount()) {
                    result = access_binding(binding.mBinding, callback);
                    block->decreaseStrongCount();
                }
                return result;
            }

            const ControlPtr &ptr() const
            {
                return mPtr;
            }

        private:
            ControlPtr mPtr;
            Binding mBinding;
        };

        template <AnyBinding Binding>
        BindingPoint<Binding> bind(Binding &&binding)
        {
            return { mPtr, std::forward<Binding>(binding) };
        }

        using is_sender = void;

        using result_type = void;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(connect_t, Lifetime &lifetime, Rec &&rec)
        {
            return virtual_state<Rec>(std::forward<Rec>(rec), lifetime);
        }

    private:

        struct ControlBlock {
            ControlBlock(state &state)
                : mState(state)
            {
            }

            ~ControlBlock()
            {
                assert(mStrongRefCount == 0);
            }

            bool increaseStrongCount()
            {
                uint32_t refCount = mStrongRefCount;
                while (refCount > 0) {
                    if (mStrongRefCount.compare_exchange_weak(refCount, refCount + 1)) {
                        return true;
                    }
                }
                return false;
            }

            void decreaseStrongCount()
            {
                if (mStrongRefCount.fetch_sub(1) == 1) {
                    mState.set_value();
                    decreaseWeakCount();
                }
            }

            void increaseWeakCount()
            {
                ++mWeakRefCount;
            }

            void decreaseWeakCount()
            {
                if (mWeakRefCount.fetch_sub(1) == 1) {
                    delete this;
                }
            }

            bool request_stop()
            {
                if (increaseStrongCount()) {
                    bool result = mState.mStopSource.request_stop();
                    decreaseStrongCount();
                    return result;
                }
                return false;
            }

            bool running() const
            {
                return mStrongRefCount > 0;
            }

            state &mState;
            std::atomic<uint32_t> mStrongRefCount = 1;
            std::atomic<uint32_t> mWeakRefCount = 1;
        };

        template <typename Sender>
        struct attach_state;

        template <typename Sender>
        struct attach_receiver {

            attach_receiver(attach_state<Sender> *state)
                : mState(state)
            {
            }

            template <typename... V>
            void set_value(V &&...)
            {
                mState->mPtr.mBlock->decreaseStrongCount();
                delete mState;
            }
            void set_done()
            {
                mState->mPtr.mBlock->decreaseStrongCount();
                delete mState;
            }
            template <typename... R>
            void set_error(R &&...)
            {
                mState->mPtr.mBlock->decreaseStrongCount();
                delete mState;
            }

            friend StopToken tag_invoke(get_stop_token_t, attach_receiver<Sender> &rec)
            {
                return rec.mState->mPtr.mBlock->mState.mStopSource.get_token();
            }

            template <typename CPO, typename... Args>
                requires(is_tag_invocable_v<CPO, state &, Args...>)
            friend auto tag_invoke(CPO f, attach_receiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, state &, Args...>)
                -> tag_invoke_result_t<CPO, state &, Args...>
            {
                return f(rec.mState->mPtr.mBlock->mState, std::forward<Args>(args)...);
            }

            attach_state<Sender> *mState;
        };

        template <typename Sender>
        struct attach_state {
            attach_state(Sender &&sender, ControlPtr ptr)
                : mPtr(std::move(ptr))
                , mState(connect(std::forward<Sender>(sender), attach_receiver<Sender> { this }))
            {
            }
            void start()
            {
                if (mPtr.mBlock->increaseStrongCount()) {
                    mState.start();
                } else {
                    delete this;
                }
            }

            ControlPtr mPtr;
            connect_result_t<Sender, attach_receiver<Sender>> mState;
        };

        struct state : VirtualReceiverBaseEx<type_pack<>, type_pack<>, cpos...>, StopCallback {

            state(Lifetime &lifetime)
                : mControl(*new ControlBlock(*this))
                , mLifetime(lifetime)
            {
                [[maybe_unused]] bool registered = mStopSource.registerCallback(this);
                assert(registered);
            }

            template <typename F>
            state(Lifetime &lifetime, F &&callback)
                : state(lifetime)
            {
                std::forward<F>(callback)(mControl);
            }

            void start()
            {
                this->mLifetime.mPtr = this->mControl;
            }

            void stop()
            {
                this->mStopSource.request_stop();
            }

            void stopRequested() override
            {
                mControl.mBlock->decreaseStrongCount();
            }

            friend auto tag_invoke(Execution::visit_state_t, state *state, auto &&visitor)
            {
                visitor(State::BeginBlock { "Lifetime" });
                if (state) {
                    visitor(State::Marker {});
                    visitor(State::Text { "Active: " + std::to_string(state->mControl.mBlock->mStrongRefCount) });
                }
                visitor(State::EndBlock {});
            }

            ControlPtr mControl;
            StopSource mStopSource;
            Lifetime &mLifetime;
        };

        ControlPtr mPtr;
    };
}
}