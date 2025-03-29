#pragma once

#include "pendingrequest.h"
#include "streamresult.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT message_streambuf : std::basic_streambuf<char> {

        message_streambuf() = default;
        message_streambuf(const message_streambuf &) = delete;

        message_streambuf &operator=(const message_streambuf &) = delete;

        void beginMessageWrite();
        void endMessageWrite(ParticipantId requester = 0, MessageId requestId = 0, GenericMessageReceiver receiver = {});

        MessageId beginMessageRead();
        std::streamsize endMessageRead();

        PendingRequest getRequest(MessageId id);

        virtual StreamResult sendMessages() = 0;
        virtual StreamResult receiveMessages() = 0;

    protected:
        virtual void beginMessageWriteImpl() = 0;
        virtual MessageId endMessageWriteImpl() = 0;

        virtual MessageId beginMessageReadImpl() = 0;
        virtual std::streamsize endMessageReadImpl();

    private:
        std::list<PendingRequest> mPendingRequests;
    };

}
}
