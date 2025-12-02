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
        struct FunctionPtr;
    }

    using StateDescriptor = std::variant<State::Text, State::Progress, State::BeginBlock, State::EndBlock, State::PushDisabled, State::PopDisabled, State::SubLocation, State::Breakpoint, State::Marker, State::FunctionPtr>;

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
        struct FunctionPtr {
            void *mPtr;
            const char *mTypeName;
        };
    }

    struct visit_state_t {
        template <typename T, typename V>
            requires(!tag_invocable<visit_state_t, T *, V>)
        auto operator()(T *, V &&visitor) const
        {
            visitor(Execution::State::Text { "Unsupported state: <"s + typeid(T).name() + ">" });
        }

        template <typename T, typename V>
            requires tag_invocable<visit_state_t, T *, V>
        auto operator()(T *t, V &&visitor) const
            noexcept(is_nothrow_tag_invocable_v<visit_state_t, T *, V>)
                -> tag_invoke_result_t<visit_state_t, T *, V>
        {
            return tag_invoke(*this, t, std::forward<V>(visitor));
        }
    };

    constexpr visit_state_t visit_state;

}
}