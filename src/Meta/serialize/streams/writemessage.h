#pragma once

#include "pendingrequest.h"

#include "serializablemapholder.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT WriteMessage {
        WriteMessage(FormattedMessageStream &stream, ParticipantId requester = 0, MessageId requestId = 0, GenericMessageReceiver receiver = {});
        WriteMessage(WriteMessage &&other);
        ~WriteMessage();

        WriteMessage &operator=(WriteMessage &&other);

        void beginHeaderWrite();
        void endHeaderWrite();

        SerializeStream &stream();

        operator FormattedMessageStream &() const;

        FormattedMessageStream *mStream;
        ParticipantId mRequester = 0;
        MessageId mRequestId = 0;
        GenericMessageReceiver mReceiver;

        SerializableMapHolder mHolder;
    };

}
}