#pragma once

#include "../typed_ptr.h"

namespace Engine {
namespace Debug {
    struct Continuation;
    struct DebugLocation;
}
namespace Execution {

    namespace State {
        struct Text {
            std::string mText;
        };
        struct Progress {
            float mRatio;
        };
        struct BeginBlock {
            std::string mName;
            bool mCompleted = false;
        };
        struct EndBlock {
        };
        struct PushDisabled {
        };
        struct PopDisabled {
        };
        struct DebugLocation {
            TypedPtr mLocation;
        };
        struct Breakpoint {
            IndexType<size_t> *mLineFeedback;
            Debug::Continuation &mContinuation;
        };
        struct Marker {
        };
        struct FunctionPtr {
            void *mId;
            void *mFunctionPtr;
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