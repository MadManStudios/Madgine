#pragma once

namespace Engine {
namespace Debug {
    struct Continuation;
}
namespace Execution {

    namespace State {
        struct Text;
        struct Progress;
        struct BeginBlock;
        struct EndBlock;
        struct PushDisabled;
        struct PopDisabled;
        struct SubLocation;
        struct Breakpoint;
        struct Marker;
    }

    using StateDescriptor = std::variant<State::Text, State::Progress, State::BeginBlock, State::EndBlock, State::PushDisabled, State::PopDisabled, State::SubLocation, State::Breakpoint, State::Marker>;

    namespace State {
        struct Text {
            std::string mText;
        };
        struct Progress {
            float mRatio;
        };
        struct BeginBlock {
            std::string mName;
        };
        struct EndBlock {
        };
        struct PushDisabled {
        };
        struct PopDisabled {
        };
        struct SubLocation {
        };
        struct Breakpoint {            
            IndexType<size_t> *mLineFeedback;
            Debug::Continuation &mContinuation;
        };
        struct Marker {
        };
    }

    struct visit_state_t {
        template <typename T, typename I, typename V>
            requires(!tag_invocable<visit_state_t, T *, const I &, V>)
        auto operator()(T *, const I &i, V &&visitor) const
        {
            visitor(Execution::State::Text { "Unsupported state: <"s + typeid(T).name() + ">" });
            if constexpr (std::same_as<I, std::string>) {
                visitor(Execution::State::Text { i });
            }
        }

        template <typename T, typename I, typename V>
            requires tag_invocable<visit_state_t, T *, const I &, V>
        auto operator()(T *t, const I &info, V &&visitor) const
            noexcept(is_nothrow_tag_invocable_v<visit_state_t, T *, const I &, V>)
                -> tag_invoke_result_t<visit_state_t, T *, const I &, V>
        {
            return tag_invoke(*this, t, info, std::forward<V>(visitor));
        }
    };

    constexpr visit_state_t visit_state;

    struct visit_sender_t {
        template <typename T>
            requires(!tag_invocable<visit_sender_t, T *>)
        auto operator()(T *) const
        {
            return std::string { "Unsupported sender: <"s + typeid(T).name() + ">" };
        }

        template <typename T>
            requires tag_invocable<visit_sender_t, T *>
        auto operator()(T *t) const
            noexcept(is_nothrow_tag_invocable_v<visit_sender_t, T *>)
                -> tag_invoke_result_t<visit_sender_t, T *>
        {
            return tag_invoke(*this, t);
        }
    };

    constexpr visit_sender_t visit_sender;

}
}