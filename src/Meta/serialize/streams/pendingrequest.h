#pragma once

#include "Generic/execution/sender.h"
#include "Generic/execution/virtualsender.h"
#include "Generic/execution/virtualstate.h"
#include "Generic/forward_capture.h"
#include "Generic/nulledptr.h"

namespace Engine {
namespace Serialize {

    struct GenericMessageReceiver {

        GenericMessageReceiver() = default;
        GenericMessageReceiver(Execution::VirtualReceiverBase<MessageResult, const void *> &receiver)
            : mPtr(&receiver)
        {
        }
        GenericMessageReceiver(GenericMessageReceiver &&) = default;
        ~GenericMessageReceiver()
        {
            assert(!mPtr);
        }

        GenericMessageReceiver &operator=(GenericMessageReceiver &&) = default;

        template <typename... V>
        void set_value(const V &...v)
        {
            if (mPtr) {
                std::tuple<const V &...> values { v... };
                mPtr->set_value(&values);
                mPtr.reset();
            }
        }
        void set_done()
        {
            if (mPtr) {
                mPtr->set_done();
                mPtr.reset();
            }
        }
        void set_error(MessageResult result)
        {
            if (mPtr) {
                mPtr->set_error(result);
                mPtr.reset();
            }
        }

        explicit operator bool() const
        {
            return mPtr;
        }

    private:
        NulledPtr<Execution::VirtualReceiverBase<MessageResult, const void *>> mPtr;
    };

    template <typename _Rec, typename... V>
    struct MessageState : Execution::VirtualReceiverBase<MessageResult, const void *> {
        using Rec = _Rec;

        MessageState(Rec &&rec)
            : mRec(std::forward<Rec>(rec))
        {
        }
        void set_value(V &&...value)
        {
            mRec.set_value(std::forward<V>(value)...);
        }
        virtual void set_value(const void *data) override final
        {
            assert(data);
            TupleUnpacker::invokeFromTuple(LIFT(mRec.set_value, this), *static_cast<const std::tuple<const V &...>*>(data));
        }
        virtual void set_done() override final
        {
            mRec.set_done();
        }
        virtual void set_error(MessageResult result) override final
        {
            mRec.set_error(result);
        }
        Rec mRec;
    };

    template <typename _Rec>
    struct MessageState<_Rec, void> : Execution::VirtualReceiverBase<MessageResult, const void *> {
        using Rec = _Rec;

        MessageState(Rec &&rec)
            : mRec(std::forward<Rec>(rec))
        {
        }
        void set_value()
        {
            mRec.set_value();
        }
        virtual void set_value(const void *data) override final
        {
            assert(data);
            mRec.set_value();
        }
        virtual void set_done() override final
        {
            mRec.set_done();
        }
        virtual void set_error(MessageResult result) override final
        {
            mRec.set_error(result);
        }
        Rec mRec;
    };

    template <typename... V, typename F, typename... Args>
    auto make_message_sender(F &&f, Args &&...args)
    {
        return Execution::make_sender<MessageResult, V...>(
            [f { forward_capture(std::forward<F>(f)) }, args = std::tuple<Args...> { std::forward<Args>(args)... }]<typename Rec>(Rec &&rec) mutable {
                return Execution::make_simple_state<MessageState<Rec, V...>>(std::forward<F>(f), std::move(args), std::forward<Rec>(rec));
            });
    }

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