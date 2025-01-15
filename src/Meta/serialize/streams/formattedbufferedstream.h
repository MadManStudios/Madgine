#pragma once

#include "formattedserializestream.h"

#include "pendingrequest.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT FormattedBufferedStream : FormattedSerializeStream {

        FormattedBufferedStream(std::unique_ptr<Serialize::Formatter> format, std::unique_ptr<message_streambuf> buffer, std::unique_ptr<SyncStreamData> data);
        FormattedBufferedStream(FormattedBufferedStream &&other) = default;
        FormattedBufferedStream(FormattedBufferedStream &&other, SyncManager *mgr);

        FormattedBufferedStream &operator=(FormattedBufferedStream &&) = default;

        void beginMessageWrite();
        void endMessageWrite();
        void endMessageWrite(ParticipantId requester, MessageId requestId, GenericMessageReceiver receiver = {});

        struct META_EXPORT MessageReadMarker {
            MessageReadMarker(MessageId id = 0, Formatter *formatter = nullptr)
                : mId(id)
                , mFormatter(formatter)
            {
            }
            MessageReadMarker(MessageReadMarker &&other) = default;
            ~MessageReadMarker();

            MessageReadMarker &operator=(MessageReadMarker &&other);

            StreamResult end();

            explicit operator bool() const;

            MessageId mId;
            Formatter *mFormatter;
        };

        StreamResult beginMessageRead(MessageReadMarker &msg);

        PendingRequest getRequest(MessageId id);

        StreamResult sendMessages();

        StreamResult beginHeaderRead();
        StreamResult endHeaderRead();

        void beginHeaderWrite();
        void endHeaderWrite();

    protected:
        message_streambuf &buffer();
    };

}
}