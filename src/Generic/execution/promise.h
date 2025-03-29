#pragma once

#include "../withresultfuture.h"
#include "execution.h"

namespace Engine {
namespace Execution {

    struct then_promise_t {

        template <typename Rec, typename P>
        struct receiver : algorithm_receiver<Rec> {

            template <typename... V>
            void set_value(V &&...value)
            {
                mPromise.set_value({ value... });
                this->mRec.set_value(std::forward<V>(value)...);
            }

            void set_done()
            {
                this->mRec.set_done();
            }

            template <typename... R>
            void set_error(R &&...result)
            {
                mPromise.set_value({ result... });
                this->mRec.set_error(std::forward<R>(result)...);
            }

            P mPromise;
        };


        template <typename Sender, typename P>
        struct sender {
            using result_type = typename Sender::result_type;
            template <typename Rec, template <typename...> typename Tuple>
            using value_types = typename Sender::template value_types<Rec, Tuple>;

            template <typename Rec>
            auto operator()(Rec &&rec)
            {
                return algorithm_state<Sender, receiver<Rec, P>> { std::forward<Sender>(mSender), std::forward<Rec>(rec), std::forward<P>(mPromise) };
            }

            Sender mSender;
            P mPromise;
        };

        template <typename Sender, typename P>
        friend auto tag_invoke(then_promise_t, Sender &&inner, P &&p)
        {
            return sender<Sender, P> { std::forward<Sender>(inner), std::forward<P>(p) };
        }

        template <typename Sender, typename P>
        requires tag_invocable<then_promise_t, Sender, P>
        auto operator()(Sender &&sender, P &&promise) const
            noexcept(is_nothrow_tag_invocable_v<then_promise_t, Sender, P>)
                -> tag_invoke_result_t<then_promise_t, Sender, P>
        {
            return tag_invoke(*this, std::forward<Sender>(sender), std::forward<P>(promise));
        }
    };

    struct detach_with_future_t {
    
        template <typename Sender>
        struct traits {
            using R = typename Sender::result_type;
            template <typename... V>
            using helper = std::promise<WithResult<R, V...>>;
            using Promise = typename Sender::template value_types<helper>;
            template <typename... V>
            using helper2 = WithResultFuture<R, V...>;
            using Future = typename Sender::template value_types<helper2>; 
        };

        template <typename Sender>
        auto operator()(Sender &&sender)
        {
            typename traits<Sender>::Promise p;
            typename traits<Sender>::Future f { p.get_future() };
            detach(then_promise(std::forward<Sender>(sender), std::move(p)));
            return f;
        }

    };
    inline constexpr detach_with_future_t detach_with_future;

}
}