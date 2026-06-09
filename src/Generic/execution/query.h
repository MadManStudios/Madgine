#pragma once

#include "concepts.h"

namespace Engine {
namespace Execution {

    struct with_query_value_t {

        template <typename Rec, typename CPO, typename T>
        struct receiver : algorithm_receiver<Rec> {

            receiver(Rec &&rec, T &&queryResult)
                : algorithm_receiver<Rec> { std::forward<Rec>(rec) }
                , mQueryResult(std::forward<T>(queryResult))
            {
            }

            friend T tag_invoke(CPO, receiver &rec)
            {
                return rec.mQueryResult;
            }

            T mQueryResult;
        };

        template <typename Sender, typename CPO, typename T>
        struct sender : algorithm_sender<Sender> {

            template <typename Rec>
            friend auto tag_invoke(connect_t, sender &&sender, Rec &&rec)
            {
                return algorithm_state<Sender, receiver<Rec, CPO, T>> { std::forward<Sender>(sender.mSender), receiver<Rec, CPO, T> { std::forward<Rec>(rec), std::forward<T>(sender.mQueryResult) } };
            }

            T mQueryResult;
        };

        template <typename Sender, typename CPO, typename T>
        friend auto tag_invoke(with_query_value_t, Sender &&inner, CPO cpo, T &&queryResult)
        {
            return sender<Sender, CPO, T> { { {}, std::forward<Sender>(inner) }, std::forward<T>(queryResult) };
        }

        template <typename Sender, typename CPO, typename T>
            requires tag_invocable<with_query_value_t, Sender, CPO, T>
        auto operator()(Sender &&sender, CPO cpo, T &&queryResult) const
            noexcept(is_nothrow_tag_invocable_v<with_query_value_t, Sender, CPO, T>)
                -> tag_invoke_result_t<with_query_value_t, Sender, CPO, T>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), cpo, std::forward<T>(queryResult));
        }

        template <typename CPO, typename T>
        auto operator()(CPO cpo, T &&queryResult) const
        {
            return pipable_from_right(*this, cpo, std::forward<T>(queryResult));
        }
    };

    inline constexpr with_query_value_t with_query_value;

}
}