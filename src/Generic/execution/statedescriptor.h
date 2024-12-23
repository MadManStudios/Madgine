#pragma once

#include "algorithm.h"

#include "../callable_view.h"

namespace Engine {
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
    }

    using StateDescriptor = std::variant<State::Text, State::Progress, State::BeginBlock, State::EndBlock, State::PushDisabled, State::PopDisabled, State::SubLocation, State::Breakpoint>;

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
            bool &mSet;
            enum class Alignment {
                Top,
                Center,
                Bottom
            } mAlignment = Alignment::Center;
        };
    }

    struct visit_state_t {
        template <typename T, typename I, typename V>
        requires(!tag_invocable<visit_state_t, T &, const I &, V>) auto operator()(T &, const I &, V &&visitor) const
        {
            visitor(Execution::State::SubLocation {});
        }

        template <typename T, typename I, typename V>
        requires tag_invocable<visit_state_t, T &, const I &, V>
        auto operator()(T &t, const I &info, V &&visitor) const
            noexcept(is_nothrow_tag_invocable_v<visit_state_t, T &, const I &, V>)
                -> tag_invoke_result_t<visit_state_t, T &, const I &, V>
        {
            return tag_invoke(*this, t, info, std::forward<V>(visitor));
        }
    };

    constexpr visit_state_t visit_state;

    struct visit_sender_t {
        template <typename T>
        requires(!tag_invocable<visit_sender_t, T &>) auto operator()(T &) const
        {
            return std::monostate {};
        }

        template <typename T>
        requires tag_invocable<visit_sender_t, T &>
        auto operator()(T &t) const
            noexcept(is_nothrow_tag_invocable_v<visit_sender_t, T &>)
                -> tag_invoke_result_t<visit_sender_t, T &>
        {
            return tag_invoke(*this, t);
        }
    };

    constexpr visit_sender_t visit_sender;

    template <typename... Sender>
    auto tag_invoke(visit_sender_t, sequence_t::sender<Sender...> &sender)
    {
        TupleUnpacker::forEach(sender.mSenders, [&](auto &sender) {
            visit_sender(sender);
        });
        return std::monostate {};
    }

}
}