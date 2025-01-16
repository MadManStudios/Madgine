#include "../../metalib.h"

#include "formattedmessagestream.h"

#include "../syncmanager.h"
#include "message_streambuf.h"
#include "syncstreamdata.h"
#include "writemessage.h"
#include "readmessage.h"

namespace Engine {
namespace Serialize {

    FormattedMessageStream::FormattedMessageStream(std::unique_ptr<Serialize::Formatter> format, std::unique_ptr<message_streambuf> buffer, std::unique_ptr<SyncStreamData> data)
        : FormattedSerializeStream(std::move(format), { std::move(buffer), std::move(data) })
    {
    }

    FormattedMessageStream::FormattedMessageStream(FormattedMessageStream &&other, SyncManager *mgr)
        : FormattedMessageStream(std::move(other))
    {
        // mFormatter->stream().data()->setManager(mgr);
    }

    WriteMessage FormattedMessageStream::beginMessageWrite(ParticipantId requester, MessageId requestId, GenericMessageReceiver receiver)
    {
        return { this, requester, requestId, std::move(receiver) };
    }

    
    StreamResult FormattedMessageStream::beginMessageRead(ReadMessage &msg)
    {
        if (!mFormatter)
            throw 0;

        MessageId id = buffer().beginMessageRead();
        if (id == 0) {
            STREAM_PROPAGATE_ERROR(buffer().receiveMessages());
            id = buffer().beginMessageRead();
        }

        if (id != 0) {
            mFormatter->stream().clear();
            STREAM_PROPAGATE_ERROR(mFormatter->beginMessageRead());
            msg = { id, mFormatter.get() };
        }
        return {};
    }

    PendingRequest FormattedMessageStream::getRequest(MessageId id)
    {
        return buffer().getRequest(id);
    }

    StreamResult FormattedMessageStream::sendMessages()
    {
        if (!mFormatter)
            throw 0;
        return buffer().sendMessages();
    }

    StreamResult ReadMessage::beginHeaderRead()
    {
        return mFormatter->beginHeaderRead();
    }

    StreamResult ReadMessage::endHeaderRead()
    {
        return mFormatter->endHeaderRead();
    }

    void WriteMessage::beginHeaderWrite()
    {
        mStream->mFormatter->beginHeaderWrite();
    }

    void WriteMessage::endHeaderWrite()
    {
        mStream->mFormatter->endHeaderWrite();
    }

    message_streambuf &FormattedMessageStream::buffer()
    {
        return static_cast<message_streambuf &>(mFormatter->stream().buffer());
    }
}
}