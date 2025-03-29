#pragma once

#include "formattedserializestream.h"

#include "pendingrequest.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT FormattedMessageStream : FormattedSerializeStream {

        FormattedMessageStream(std::unique_ptr<Serialize::Formatter> format, std::unique_ptr<message_streambuf> buffer, std::unique_ptr<SyncStreamData> data);
        FormattedMessageStream(FormattedMessageStream &&other) = default;
        FormattedMessageStream(FormattedMessageStream &&other, SyncManager *mgr);

        FormattedMessageStream &operator=(FormattedMessageStream &&) = default;

        [[nodiscard]] WriteMessage beginMessageWrite(ParticipantId requester = 0, MessageId requestId = 0, GenericMessageReceiver receiver = {});

        StreamResult beginMessageRead(ReadMessage &msg);

        PendingRequest getRequest(MessageId id);

        StreamResult sendMessages();

        friend struct WriteMessage;

    protected:
        message_streambuf &buffer();
    };

}
}