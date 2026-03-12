#pragma once

#include "virtualstate.h"

namespace Engine {
namespace Execution {

    template <typename R, typename... Ty>
    using ConnectionReceiver = VirtualReceiverBaseEx<to_type_pack<R>, type_pack<Ty...>>;

    template <typename Stub>
    struct Connection : Stub::template value_types<type_pack>::template prepend<typename Stub::result_type>::template instantiate<ConnectionReceiver> {

        Connection(Stub &stub)
            : mStub(stub)
        {
        }

        void start()
        {
            mStub.enqueue(this);
        }

        void stop()
        {
            if (mStub.extract(this))
                this->set_done();
        }

        friend auto tag_invoke(visit_state_t, Connection *con, auto &&visitor)
        {
            if (con) {
                visitor(State::Marker {});
            }
            visitor(State::Text { typeid(Stub).name() });
        }

    protected:
        template <typename>
        friend struct ConnectionStack;
        template <typename>
        friend struct ConnectionQueue;

        Stub &mStub;
        std::atomic<Connection<Stub> *> mNext = nullptr;
    };

    template <typename Stub, typename R, typename... Ty>
    struct ConnectionSender {

        using result_type = R;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<Ty...>;

        using is_sender = void;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, Stub &stub, Rec &&rec)
        {
            return Execution::VirtualState<Connection<Stub>, Rec>(std::forward<Rec>(rec), stub);
        }
    };

}
}
