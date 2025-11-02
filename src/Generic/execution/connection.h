#pragma once

#include "virtualstate.h"

namespace Engine {
namespace Execution {

    template <typename... Ty>
    using ConnectionReceiver = VirtualReceiverBaseEx<type_pack<>, type_pack<Ty...>>;

    template <typename Stub, typename... Ty>
    struct Connection : ConnectionReceiver<Ty...> {

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
                
        friend auto tag_invoke(visit_state_t, Connection *con, const auto &info, auto &&visitor)
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
        std::atomic<Connection<Stub, Ty...> *> mNext = nullptr;
    };

    template <typename Stub, typename... Ty>
    struct ConnectionSender {

        using result_type = void;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<Ty...>;

        using is_sender = void;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, Stub &stub, Rec &&rec)
        {
            return Execution::VirtualState<Connection<Stub, Ty...>, Rec>(std::forward<Rec>(rec), stub);
        }
    };

}
}
