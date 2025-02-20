#pragma once

#include "debuglocation.h"

#include "Generic/execution/concepts.h"

#include "debuggablesender.h"

namespace Engine {
namespace Debug {

    struct MADGINE_DEBUGGER_EXPORT DebugLocationSplitter : DebugLocation {

        DebugLocationSplitter(size_t channelCount);
        DebugLocationSplitter(auto &&, size_t channelCount) : DebugLocationSplitter(channelCount) { }

        ParentLocation *channel(size_t index);

        const std::vector<ParentLocation> &channels() const;

        void stepInto(ParentLocation *parent);

        std::string toString() const override;
        std::map<std::string_view, ValueType> localVariables() const override;
        bool wantsPause(ContinuationType type) const override;

    private:
        std::vector<ParentLocation> mChildLocations;
    };

    struct debug_channel_t {

        template <typename Rec>
        struct receiver : Execution::algorithm_receiver<Rec> {

            friend ParentLocation *tag_invoke(Execution::get_debug_location_t, receiver &rec)
            {
                return Execution::get_debug_location(rec.mRec)->channel(rec.mIndex);
            }

            size_t mIndex;
        };

        template <typename Sender>
        struct sender : Execution::algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(Execution::connect_t, sender &&sender, Rec &&rec)
            {
                return Execution::algorithm_state<Sender, receiver<Rec>> { std::forward<Sender>(sender.mSender), std::forward<Rec>(rec), sender.mIndex };
            }

            size_t mIndex;
        };

        template <typename Sender>
        friend auto tag_invoke(debug_channel_t, Sender &&inner, size_t index)
        {
            return sender<Sender> { { {}, std::forward<Sender>(inner) }, index };
        }

        template <typename Sender>
            requires tag_invocable<debug_channel_t, Sender, size_t>
        auto operator()(Sender &&sender, size_t index) const
            noexcept(is_nothrow_tag_invocable_v<debug_channel_t, Sender, size_t>)
                -> tag_invoke_result_t<debug_channel_t, Sender, size_t>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), index);
        }

        auto operator()(size_t index) const
        {
            return pipable_from_right(*this, std::move(index));
        }
    };

    inline constexpr debug_channel_t debug_channel;

}
}