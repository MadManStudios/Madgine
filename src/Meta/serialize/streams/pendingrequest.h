#pragma once

#include "Generic/execution/future.h"
#include "Generic/nulledptr.h"

namespace Engine {
namespace Serialize {

    struct GenericMessageReceiver {

        GenericMessageReceiver() = default;
        template <typename... T>
        GenericMessageReceiver(Execution::Promise<MessageResult, T...> promise)
            : mPromise(std::move(reinterpret_cast<Execution::Promise<MessageResult> &>(promise)))
        {
        }
        GenericMessageReceiver(GenericMessageReceiver &&) = default;
        ~GenericMessageReceiver()
        {
        }

        GenericMessageReceiver &operator=(GenericMessageReceiver &&) = default;

        template <typename... V, typename... V2>
        void set_value(V2 &&...v)
        {
            if (mPromise)
                reinterpret_cast<Execution::Promise<MessageResult, V...> &>(mPromise).set_value(std::forward<V2>(v)...);
        }
        void set_done()
        {
            if (mPromise)
                mPromise.set_done();
        }
        void set_error(MessageResult result)
        {
            if (mPromise)
                mPromise.set_error(std::move(result));
        }

        explicit operator bool() const
        {
            return static_cast<bool>(mPromise);
        }

    private:
        Execution::Promise<MessageResult> mPromise = std::nullopt;
    };

    struct PendingRequest {

        PendingRequest(MessageId id = 0, ParticipantId requester = 0, MessageId requesterTransactionId = 0, GenericMessageReceiver receiver = {})
            : mId(id)
            , mRequester(requester)
            , mRequesterTransactionId(requesterTransactionId)
            , mReceiver(std::move(receiver))
        {
        }

        PendingRequest(PendingRequest &&other)
            : mId(std::exchange(other.mId, 0))
            , mRequester(std::exchange(other.mRequester, 0))
            , mRequesterTransactionId(std::exchange(other.mRequesterTransactionId, 0))
            , mReceiver(std::move(other.mReceiver))
        {
        }

        ~PendingRequest()
        {
        }

        PendingRequest &operator=(PendingRequest &&other)
        {
            mId = std::exchange(other.mId, 0);
            mRequester = std::exchange(other.mRequester, 0);
            mRequesterTransactionId = std::exchange(other.mRequesterTransactionId, 0);
            std::swap(mReceiver, other.mReceiver);
            return *this;
        }

        MessageId mId = 0;
        ParticipantId mRequester = 0;
        MessageId mRequesterTransactionId = 0;
        GenericMessageReceiver mReceiver;
    };

}
}