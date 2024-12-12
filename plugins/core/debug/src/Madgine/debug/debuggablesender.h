#pragma once

#include "Generic/execution/statedescriptor.h"
#include "debugger.h"
//#include "Meta/keyvalue/valuetype.h"

namespace Engine {
namespace Execution {

    struct MADGINE_DEBUGGER_EXPORT SenderLocation : Debug::DebugLocation {
        SenderLocation(std::vector<StateDescriptor> state);

        std::string toString() const override;
        std::map<std::string_view, ValueType> localVariables() const override;
        bool wantsPause(Debug::ContinuationType type) const override;

        std::vector<StateDescriptor> mState;
        size_t mIndex = 0;
        const void *mContextData = nullptr;
    };

    struct get_debug_location_t {

        template <typename T>
        requires(!tag_invocable<get_debug_location_t, T &>) auto operator()(T &t) const
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

    struct get_debug_data_t {

        template <typename T>
        requires(!tag_invocable<get_debug_data_t, T &>) auto operator()(T &t) const
        {
            return nullptr;
        }

        template <typename T>
        requires tag_invocable<get_debug_data_t, T &>
        auto operator()(T &t) const
            noexcept(is_nothrow_tag_invocable_v<get_debug_data_t, T &>)
                -> tag_invoke_result_t<get_debug_data_t, T &>
        {
            return tag_invoke(*this, t);
        }
    };

    inline constexpr get_debug_data_t get_debug_data;

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

            friend SenderLocation *tag_invoke(get_debug_location_t, receiver &rec)
            {
                return &rec.mState->mLocation;
            }

            template <typename CPO, typename... Args>
            requires(is_tag_invocable_v<CPO, Rec &, Args...>) friend auto tag_invoke(CPO f, receiver &rec, Args &&...args) noexcept(is_nothrow_tag_invocable_v<CPO, Rec &, Args...>)
                -> tag_invoke_result_t<CPO, Rec &, Args...>
            {
                return tag_invoke(f, rec.mState->mRec, std::forward<Args>(args)...);
            }

            state<Sender, Rec> *mState;
        };

        template <typename Sender, typename _Rec>
        struct state {
            using Rec = _Rec;

            using State = connect_result_t<Sender, receiver<Sender, Rec>>;

            state(Sender &&sender, Rec &&rec, std::vector<StateDescriptor> state)
                : mRec(std::forward<Rec>(rec))
                , mLocation(std::move(state))
                , mState { connect(std::forward<Sender>(sender), receiver<Sender, Rec> { this }) }
            {
            }

            ~state() { }

            void start()
            {
                mLocation.mIndex = 0;
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

            Rec mRec;
            SenderLocation mLocation;
            State mState;

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, state &state, Args &&...args)
                -> tag_invoke_result_t<CPO, State &, Args...>
            {
                return f(state.mState, std::forward<Args>(args)...);
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, const state &state, Args &&...args)
                -> tag_invoke_result_t<CPO, const State &, Args...>
            {
                return f(state.mState, std::forward<Args>(args)...);
            }
        };

        template <typename Sender>
        struct sender : algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                std::vector<StateDescriptor> stateDesc;
                auto visitor = [&](const StateDescriptor &desc) {
                    stateDesc.push_back(desc);
                };
                visit_state(sender, CallableView<void(const StateDescriptor &desc)> { visitor });
                return state<Sender, Rec> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec), std::move(stateDesc) };
            }
        };

        template <Sender Sender>
        friend auto tag_invoke(with_debug_location_t, Sender &&inner)
        {
            return sender<Sender> { { {}, std::forward<Sender>(inner) } };
        }

        template <Sender Sender>
        requires tag_invocable<with_debug_location_t, Sender>
        auto operator()(Sender &&sender) const
            noexcept(is_nothrow_tag_invocable_v<with_debug_location_t, Sender>)
                -> tag_invoke_result_t<with_debug_location_t, Sender>
        {
            return tag_invoke(*this, std::forward<Sender>(sender));
        }

        auto operator()() const
        {
            return pipable_from_right(*this);
        }
    };

    inline constexpr with_debug_location_t with_debug_location;

    inline constexpr auto with_sub_debug_location = [](auto *location) {
        return with_query_value(get_debug_location, std::move(location));
    };

    template <typename Sender>
    using DebuggableSender = decltype(std::declval<Sender>() | with_debug_location());

    template <typename T>
    concept is_debuggable = tag_invocable<get_debug_location_t, T &>;

    struct tracked_t {

        template <is_debuggable Rec, typename Sender>
        struct state;

        template <is_debuggable Rec, size_t operation_increment, size_t stop_increment>
        struct receiver : algorithm_receiver<Rec> {

            template <typename... V>
            void set_value(V &&...value)
            {
                if constexpr (operation_increment == 0 && stop_increment == 0) {
                    this->mRec.set_value(std::forward<V>(value)...);
                } else {
                    assert(dynamic_cast<SenderLocation *>(static_cast<Debug::DebugLocation *>(get_debug_location(this->mRec))));
                    SenderLocation *location = static_cast<SenderLocation *>(get_debug_location(this->mRec));

                    location->mIndex += operation_increment;

                    location->pass([=](Debug::ContinuationMode mode, V &&...value) mutable {
                        location->mIndex += stop_increment;
                        switch (mode) {
                        case Debug::ContinuationMode::Resume:
                        case Debug::ContinuationMode::Step:
                            this->mRec.set_value(std::forward<V>(value)...);
                            break;
                        case Debug::ContinuationMode::Abort:
                            this->mRec.set_done();
                            break;
                        }
                    },
                        get_stop_token(this->mRec), Debug::ContinuationType::Return, std::forward<V>(value)...);
                }
            }

            void set_done()
            {
                if constexpr (operation_increment == 0 && stop_increment == 0) {
                    this->mRec.set_done();
                } else {
                    SenderLocation *location = get_debug_location(this->mRec);

                    location->mIndex += operation_increment;

                    location->pass([=](Debug::ContinuationMode mode) {
                        location->mIndex += stop_increment;
                        this->mRec.set_done();
                    },
                        get_stop_token(this->mRec), Debug::ContinuationType::Cancelled);
                }
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                if constexpr (operation_increment == 0 && stop_increment == 0) {
                    this->mRec.set_error(std::forward<R>(result)...);
                } else {
                    SenderLocation *location = get_debug_location(this->mRec);

                    location->mIndex += operation_increment;

                    location->pass([=](Debug::ContinuationMode mode, R &&...result) mutable {
                        location->mIndex += stop_increment;
                        switch (mode) {
                        case Debug::ContinuationMode::Resume:
                        case Debug::ContinuationMode::Step:
                            this->mRec.set_error(std::forward<R>(result)...);
                            break;
                        case Debug::ContinuationMode::Abort:
                            this->mRec.set_done();
                            break;
                        }
                    },
                        get_stop_token(this->mRec), Debug::ContinuationType::Error, std::forward<R>(result)...);
                }
            }
        };

        template <is_debuggable _Rec, typename Sender>
        struct state {
            using Rec = _Rec;

            using State = tag_invoke_result_t<connect_t, Sender, receiver<Rec, get_debug_operation_increment<Sender>(), get_debug_stop_increment<Sender>()>>;

            state(Rec &&rec, Sender &&sender)
                : mState { tag_invoke(connect_t {}, std::forward<Sender>(sender), receiver<Rec, get_debug_operation_increment<Sender>(), get_debug_stop_increment<Sender>()> { std::forward<Rec>(rec) }) }
            {
            }

            ~state() { }

            void start()
            {
                auto location = get_debug_location(get_receiver(mState));
                if constexpr (std::same_as<SenderLocation *, decltype(location)>)
                    location->mContextData = get_debug_data(mState);

                constexpr size_t increment = get_debug_start_increment<Sender>();
                if constexpr (increment == 0) {
                    mState.start();
                } else {
                    location->pass([=](Debug::ContinuationMode mode) {
                        location->mIndex += increment;
                        mState.start();
                    },
                        get_stop_token(get_receiver(mState)), Debug::ContinuationType::Flow);
                }
            }

            State mState;

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, state &state, Args &&...args)
                -> tag_invoke_result_t<CPO, State &, Args...>
            {
                return f(state.mState, std::forward<Args>(args)...);
            }

            template <typename CPO, typename... Args>
            friend auto tag_invoke(CPO f, const state &state, Args &&...args)
                -> tag_invoke_result_t<CPO, const State &, Args...>
            {
                return f(state.mState, std::forward<Args>(args)...);
            }
        };

        template <typename Sender>
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
    requires(tag_invocable<connect_t, wrap<Sender>, Rec>) auto tag_invoke(outer_connect_t, Sender &&sender, Rec &&rec) noexcept(is_nothrow_tag_invocable_v<connect_t, wrap<Sender>, Rec>)
        -> tag_invoke_result_t<connect_t, wrap<Sender>, Rec>
    {
        return tag_invoke(connect_t {}, tracked(std::forward<Sender>(sender)), std::forward<Rec>(rec));
    }

}
}