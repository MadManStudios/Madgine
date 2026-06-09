#pragma once

#include "contextinfo.h"
#include "debuglocation.h"
#include "senderlocation.h"

namespace Engine {
namespace Execution {

    struct get_debug_location_t {

        using signature = Debug::SenderLocation *();

        template <typename T>
            requires(!tag_invocable<get_debug_location_t, T &>)
        auto operator()(T &t) const
        {
            return nullptr;
        }

        template <typename T>
            requires tag_invocable<get_debug_location_t, T &>
        auto operator()(T &t) const
            noexcept(is_nothrow_tag_invocable_v<get_debug_location_t, T &>)
                -> tag_invoke_result_t<get_debug_location_t, T &>
        {
            return tag_invoke(*this, t);
        }
    };

    inline constexpr get_debug_location_t get_debug_location;

    struct with_debug_location_t {

        template <typename Sender, typename _Rec>
        struct state;

        template <typename Sender, typename Rec>
        struct receiver {

            template <typename... V>
            void set_value(V &&...value)
            {
                mState->set_value(std::forward<V>(value)...);
            }

            void set_done()
            {
                mState->set_done();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState->set_error(std::forward<R>(result)...);
            }

            friend Debug::SenderLocation *tag_invoke(get_debug_location_t, receiver &rec)
            {
                return &rec.mState->mLocation;
            }

            template <typename CPO, typename... Args>
                requires(is_tag_invocable_v<CPO, Rec &, Args...>)
            friend auto tag_invoke(CPO f, receiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, Rec &, Args...>)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return tag_invoke(f, rec.mState->mRec, std::forward<Args>(args)...);
            }

            state<Sender, Rec> *mState;
        };

        template <typename Sender, typename _Rec>
        struct state {
            using InnerRec = _Rec;
            using Rec = receiver<Sender, InnerRec>;

            using State = connect_result_t<Sender, Rec>;

            state(Sender &&sender, InnerRec &&rec, Debug::SenderLocation *&parent)
                : mRec(std::forward<InnerRec>(rec))
                , mLocation([this](CallableView<void(const Execution::StateDescriptor &)> visitor) { visit_state(&mState, std::move(visitor)); })
                , mState { connect(std::forward<Sender>(sender), Rec { this }) }
                , mParent(parent)
            {
            }

            ~state() { }

            void start()
            {
                mLocation.stepInto(mParent, Debug::get_debug_context(mRec));
                mState.start();
            }

            void stop()
            {
                mState.stop();
            }

            template <typename... V>
            void set_value(V &&...value)
            {
                mLocation.stepOut(mParent, Debug::get_debug_context(mRec));
                mRec.set_value(std::forward<V>(value)...);
            }

            void set_done()
            {
                mLocation.stepOut(mParent, Debug::get_debug_context(mRec));
                mRec.set_done();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mLocation.stepOut(mParent, Debug::get_debug_context(mRec));
                mRec.set_error(std::forward<R>(result)...);
            }

            /* friend auto tag_invoke(Execution::visit_state_t, state &state, const auto &info, auto &&visitor, bool running)
            {
                Execution::visit_state(state.mState, info, std::forward<decltype(visitor)>(visitor), running);
            }*/

            InnerRec mRec;
            Debug::SenderLocation mLocation;
            State mState;
            Debug::SenderLocation *&mParent;
        };

        template <AnySender Sender>
        struct sender : algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Sender, Rec> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec), sender.mParent };
            }

            Debug::SenderLocation *&mParent;
        };

        template <AnySender Sender>
        friend auto tag_invoke(with_debug_location_t, Sender &&inner, Debug::SenderLocation *&parent)
        {
            return sender<Sender> { { {}, std::forward<Sender>(inner) }, parent };
        }

        template <AnySender Sender>
            requires tag_invocable<with_debug_location_t, Sender, Debug::SenderLocation *&>
        auto operator()(Sender &&sender, Debug::SenderLocation *&parent) const
            noexcept(is_nothrow_tag_invocable_v<with_debug_location_t, Sender, Debug::SenderLocation *&>)
                -> tag_invoke_result_t<with_debug_location_t, Sender, Debug::SenderLocation *&>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), parent);
        }

        auto operator()(Debug::SenderLocation *&parent) const
        {
            return pipable_from_right(*this, parent);
        }
    };

    inline constexpr with_debug_location_t with_debug_location;

    template <typename T>
    concept is_debuggable = std::same_as<tag_invoke_result_t<get_debug_location_t, T &>, Debug::SenderLocation *>;

    struct tracked_t {

        template <typename Sender, is_debuggable Rec>
        struct state;

        template <typename Sender, is_debuggable Rec>
        struct receiver {

            template <typename... V>
            void set_value(V &&...value)
            {
                mState.mPausedAtStart = false;

                Debug::SenderLocation *location = get_debug_location(this->mState.mRec);

                Debug::get_debug_context(this->mState.mRec).pass(location, this->mState.mRec, [](Rec &rec, V &&...value) mutable { rec.set_value(std::forward<V>(value)...); }, mState.mContinuation, Debug::ContinuationType::Return, mState.mEndBreakpoint, std::forward<V>(value)...);
            }

            void set_done()
            {
                mState.mPausedAtStart = false;

                Debug::SenderLocation *location = get_debug_location(this->mState.mRec);

                Debug::get_debug_context(this->mState.mRec).pass(location, this->mState.mRec, [](Rec &rec) { rec.set_done(); }, mState.mContinuation, Debug::ContinuationType::Cancelled, mState.mEndBreakpoint);
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mState.mPausedAtStart = false;

                Debug::SenderLocation *location = get_debug_location(this->mState.mRec);

                Debug::get_debug_context(this->mState.mRec).pass(location, this->mState.mRec, [](Rec &rec, R &&...result) mutable { rec.set_error(std::forward<R>(result)...); }, mState.mContinuation, Debug::ContinuationType::Error, mState.mEndBreakpoint, std::forward<R>(result)...);
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, receiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return f(rec.mState.mRec, std::forward<Args>(args)...);
            }

            state<Sender, Rec> &mState;
        };

        template <typename Sender, is_debuggable Rec>
        struct state {

            using State = tag_invoke_result_t<connect_t, Sender, receiver<Sender, Rec>>;

            state(Rec &&rec, Sender &&sender)
                : mRec(std::forward<Rec>(rec))
                , mState { tag_invoke(connect_t {}, std::forward<Sender>(sender), receiver<Sender, Rec> { *this }) }
            {
            }

            ~state() { }

            void start()
            {
                mPausedAtStart = true;

                Debug::SenderLocation *location = get_debug_location(mRec);

                Debug::get_debug_context(mRec).pass(location, mRec, [this](Rec &rec) { mState.start(); }, mContinuation, Debug::ContinuationType::Flow, mStartBreakpoint);
            }

            void stop()
            {
                // TODO proper syncing with stop_source
                if (mContinuation) {
                    mContinuation(Debug::ContinuationMode::Abort);
                } else {
                    mState.stop();
                }
            }

            friend auto tag_invoke(visit_state_t, state *state, auto &&visitor)
            {
                Debug::Continuation empty;
                visitor(Execution::State::Breakpoint {
                    state ? &state->mStartBreakpoint : nullptr,
                    state && state->mPausedAtStart ? state->mContinuation : empty });
                visit_state(state && !state->mContinuation ? &state->mState : nullptr, std::forward<decltype(visitor)>(visitor));
                visitor(Execution::State::Breakpoint {
                    state ? &state->mEndBreakpoint : nullptr,
                    state && !state->mPausedAtStart ? state->mContinuation : empty });
            }

            Rec mRec;
            State mState;

            IndexType<size_t> mStartBreakpoint;
            IndexType<size_t> mEndBreakpoint;
            bool mPausedAtStart;
            Debug::Continuation mContinuation;
        };

        template <AnySender Sender>
        struct sender : algorithm_sender<Sender> {

            template <is_debuggable Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Sender, Rec> { std::forward<Rec>(rec), std::forward<Sender>(sender.mSender) };
            }

            template <is_debuggable Rec>
            friend auto tag_invoke(connect_t, sender &sender, Rec &&rec)
            {
                return state<Sender &, Rec> { std::forward<Rec>(rec), sender.mSender };
            }
        };

        template <typename Sender>
        friend auto tag_invoke(tracked_t, Sender &&inner)
        {
            return sender<Sender> { { {}, std::forward<Sender>(inner) } };
        }

        template <typename Sender>
            requires tag_invocable<tracked_t, Sender>
        auto operator()(Sender &&sender) const
            noexcept(is_nothrow_tag_invocable_v<tracked_t, Sender>)
                -> tag_invoke_result_t<tracked_t, Sender>
        {
            return tag_invoke(*this, std::forward<Sender>(sender));
        }
    };

    inline constexpr tracked_t tracked;

    template <typename Sender>
    using wrap = tracked_t::sender<Sender>;

    template <typename Sender, is_debuggable Rec>
        requires(tag_invocable<connect_t, wrap<Sender>, Rec>)
    auto tag_invoke(outer_connect_t, Sender &&sender, Rec &&rec) noexcept(is_nothrow_tag_invocable_v<connect_t, wrap<Sender>, Rec>)
        -> tag_invoke_result_t<connect_t, wrap<Sender>, Rec>
    {
        return tag_invoke(connect_t {}, tracked(std::forward<Sender>(sender)), std::forward<Rec>(rec));
    }

}
}