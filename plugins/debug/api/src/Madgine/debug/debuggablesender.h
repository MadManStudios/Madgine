#pragma once

#include "senderlocation.h"
// #include "Meta/keyvalue/valuetype.h"

namespace Engine {
namespace Execution {

    struct get_debug_location_t {

        using signature = Debug::ParentLocation *();

        template <typename T>
            requires(!tag_invocable<get_debug_location_t, T &>)
        auto operator()(T &t) const
        {
            static_assert(std::same_as<T, void>);
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

    template <typename T>
    constexpr size_t get_debug_start_increment()
    {
        if constexpr (requires { T::debug_start_increment; }) {
            return T::debug_start_increment;
        } else {
            return 0;
        }
    }

    template <typename T>
    constexpr size_t get_debug_operation_increment()
    {
        if constexpr (requires { T::debug_operation_increment; }) {
            return T::debug_operation_increment;
        } else {
            return 0;
        }
    }

    template <typename T>
    constexpr size_t get_debug_stop_increment()
    {
        if constexpr (requires { T::debug_stop_increment; }) {
            return T::debug_stop_increment;
        } else {
            return 0;
        }
    }

    template <typename Location>
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

            friend Location *tag_invoke(get_debug_location_t, receiver &rec)
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

            template <typename... Args>
            state(Sender &&sender, InnerRec &&rec, Args&&... args)
                : mRec(std::forward<InnerRec>(rec))
                , mLocation([this, info { visit_sender(sender) }](CallableView<void(const Execution::StateDescriptor &)> visitor) { visit_state(mState, info, std::move(visitor)); }, std::forward<Args>(args)...)
                , mState { connect(std::forward<Sender>(sender), Rec { this }) }
            {
            }

            ~state() { }

            void start()
            {
                mLocation.stepInto(get_debug_location(mRec));
                mState.start();
            }

            template <typename... V>
            void set_value(V &&...value)
            {
                mLocation.stepOut(get_debug_location(mRec));
                mRec.set_value(std::forward<V>(value)...);
            }

            void set_done()
            {
                mLocation.stepOut(get_debug_location(mRec));
                mRec.set_done();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mLocation.stepOut(get_debug_location(mRec));
                mRec.set_error(std::forward<R>(result)...);
            }

            InnerRec mRec;
            Location mLocation;
            State mState;
        };

        template <Sender Sender, typename... Args>
        struct sender : algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return TupleUnpacker::constructExpand<state<Sender, Rec>>(std::forward<Sender>(sender.mSender), std::forward<Rec>(rec), std::move(sender.mArgs));
            }

            std::tuple<Args...> mArgs;
        };

        template <Sender Sender, typename... Args>
        friend auto tag_invoke(with_debug_location_t, Sender &&inner, Args&&... args)
        {
            return sender<Sender, Args...> { { {}, std::forward<Sender>(inner) }, { std::forward<Args>(args)... } };
        }

        template <Sender Sender, typename... Args>
            requires tag_invocable<with_debug_location_t, Sender, Args...>
        auto operator()(Sender &&sender, Args &&... args) const
            noexcept(is_nothrow_tag_invocable_v<with_debug_location_t, Sender, Args...>)
                -> tag_invoke_result_t<with_debug_location_t, Sender, Args...>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<Args>(args)...);
        }

        template <typename... Args>
        auto operator()(Args&&... args) const
        {
            return pipable_from_right(*this, std::forward<Args>(args)...);
        }
    };

    template <typename Location>
    inline constexpr with_debug_location_t<Location> with_debug_location;

    inline constexpr auto with_sub_debug_location = [](auto *location) {
        return with_query_value(get_debug_location, std::move(location));
    };

    template <typename T>
    concept is_debuggable = tag_invocable<get_debug_location_t, T &>;

    struct tracked_t {

        template <is_debuggable Rec, typename Sender>
        struct state;

        template <is_debuggable Rec, typename Sender>
        struct receiver {

            static constexpr size_t operation_increment = get_debug_operation_increment<Sender>();
            static constexpr size_t stop_increment = get_debug_stop_increment<Sender>();

            template <typename... V>
            void set_value(V &&...value)
            {
                if constexpr (operation_increment == 0 && stop_increment == 0) {
                    this->mState.mRec.set_value(std::forward<V>(value)...);
                } else {
                    assert(dynamic_cast<Debug::SenderLocation *>(static_cast<Debug::DebugLocation *>(get_debug_location(this->mState.mRec))));
                    Debug::SenderLocation *location = static_cast<Debug::SenderLocation *>(get_debug_location(this->mState.mRec));

                    location->mIndex += operation_increment;

                    location->pass([=, this](Debug::ContinuationMode mode, V &&...value) mutable {
                        location->mIndex += stop_increment;
                        switch (mode) {
                        case Debug::ContinuationMode::Continue:
                            this->mState.mRec.set_value(std::forward<V>(value)...);
                            break;
                        case Debug::ContinuationMode::Abort:
                            this->mState.mRec.set_done();
                            break;
                        }
                    },
                        get_stop_token(this->mState.mRec), Debug::ContinuationType::Return, false, std::forward<V>(value)...);
                }
            }

            void set_done()
            {
                if constexpr (operation_increment == 0 && stop_increment == 0) {
                    this->mState.mRec.set_done();
                } else {
                    Debug::SenderLocation *location = get_debug_location(this->mState.mRec);

                    location->mIndex += operation_increment;

                    location->pass([=, this](Debug::ContinuationMode mode) {
                        location->mIndex += stop_increment;
                        this->mState.mRec.set_done();
                    },
                        get_stop_token(this->mState.mRec), Debug::ContinuationType::Cancelled);
                }
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                if constexpr (operation_increment == 0 && stop_increment == 0) {
                    this->mState.mRec.set_error(std::forward<R>(result)...);
                } else {
                    Debug::SenderLocation *location = get_debug_location(this->mState.mRec);

                    location->mIndex += operation_increment;

                    location->pass([=, this](Debug::ContinuationMode mode, R &&...result) mutable {
                        location->mIndex += stop_increment;
                        switch (mode) {
                        case Debug::ContinuationMode::Continue:
                            this->mState.mRec.set_error(std::forward<R>(result)...);
                            break;
                        case Debug::ContinuationMode::Abort:
                            this->mState.mRec.set_done();
                            break;
                        }
                    },
                        get_stop_token(this->mState.mRec), Debug::ContinuationType::Error, false, std::forward<R>(result)...);
                }
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, receiver &rec, Args &&...args)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return f(rec.mState.mRec, std::forward<Args>(args)...);
            }

            state<Rec, Sender> &mState;
            bool mBreakpointSet = false;
        };

        template <is_debuggable _Rec, typename Sender>
        struct state {
            using Rec = _Rec;

            using State = tag_invoke_result_t<connect_t, Sender, receiver<Rec, Sender>>;

            state(Rec &&rec, Sender &&sender)
                : mRec(std::forward<Rec>(rec))
                , mState { tag_invoke(connect_t {}, std::forward<Sender>(sender), receiver<Rec, Sender> { *this }) }

            {
            }

            ~state() { }

            void start()
            {
                auto location = get_debug_location(mRec);

                constexpr size_t increment = get_debug_start_increment<Sender>();
                if constexpr (increment == 0) {
                    mState.start();
                } else {
                    location->pass([=, this](Debug::ContinuationMode mode) {
                        location->mIndex += increment;
                        mState.start();
                    },
                        get_stop_token(mRec), Debug::ContinuationType::Flow, mBreakpointSet);
                }
            }

            friend auto tag_invoke(visit_state_t, state &state, const auto &info, auto &&visitor)
            {
                visitor(Execution::State::Breakpoint { state.mBreakpointSet, Execution::State::Breakpoint::Alignment::Top });
                visit_state(state.mState, info, std::forward<decltype(visitor)>(visitor));
                // visitor(Execution::State::Breakpoint { mRec.mBreakpointSet, Execution::State::Breakpoint::Alignment::Bottom });
            }

            Rec mRec;
            State mState;

            bool mBreakpointSet = false;
        };

        template <Sender Sender>
        struct sender : algorithm_sender<Sender> {

            template <is_debuggable Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return state<Rec, Sender> { std::forward<Rec>(rec), std::forward<Sender>(sender.mSender) };
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