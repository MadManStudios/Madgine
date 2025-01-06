#include "../../metalib.h"

#include "formattedbufferedstream.h"

#include "../syncmanager.h"
#include "buffered_streambuf.h"
#include "syncstreamdata.h"

namespace Engine {
namespace Serialize {

    FormattedBufferedStream::FormattedBufferedStream(std::unique_ptr<Serialize::Formatter> format, std::unique_ptr<message_streambuf> buffer, std::unique_ptr<SyncStreamData> data)
        : FormattedSerializeStream(std::move(format), { std::move(buffer), std::move(data) })
    {
    }

    FormattedBufferedStream::FormattedBufferedStream(FormattedBufferedStream &&other, SyncManager *mgr)
        : FormattedBufferedStream(std::move(other))
    {
        //mFormatter->stream().data()->setManager(mgr);
    }

    void FormattedBufferedStream::beginMessageWrite()
    {
        buffer().beginMessageWrite();
        mFormatter->beginMessageWrite();
    }

    void FormattedBufferedStream::endMessageWrite()
    {
        mFormatter->endMessageWrite();
        buffer().endMessageWrite();
    }

    void FormattedBufferedStream::endMessageWrite(ParticipantId requester, MessageId requestId, GenericMessageReceiver receiver)
    {
        mFormatter->endMessageWrite();
        buffer().endMessageWrite(requester, requestId, std::move(receiver));
    }

    StreamResult FormattedBufferedStream::beginMessageRead(MessageReadMarker &msg)
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

    FormattedBufferedStream::MessageReadMarker::~MessageReadMarker()
    {
    }

    FormattedBufferedStream::MessageReadMarker &FormattedBufferedStream::MessageReadMarker::operator=(MessageReadMarker &&other)
    {
        assert(!mId);
        mId = std::exchange(other.mId, 0);
        mFormatter = std::move(other.mFormatter);
        return *this;
    }

    StreamResult FormattedBufferedStream::MessageReadMarker::end()
    {
        assert(mId);
        STREAM_PROPAGATE_ERROR(mFormatter->endMessageRead());
        STREAM_PROPAGATE_ERROR(mFormatter->stream().skipWs());
        mFormatter->stream().clear();
        if (static_cast<message_streambuf &>(mFormatter->stream().buffer()).endMessageRead() > 0) {
            printf("Message not fully read!");
        }
        mFormatter = nullptr;
        mId = 0;
        return {};
    }

    FormattedBufferedStream::MessageReadMarker::operator bool() const
    {
        return mId != 0;
    }

    PendingRequest FormattedBufferedStream::getRequest(MessageId id)
    {
        return buffer().getRequest(id);
    }

    StreamResult FormattedBufferedStream::sendMessages()
    {
        if (!mFormatter)
            throw 0;
        return buffer().sendMessages();
    }

    StreamResult FormattedBufferedStream::beginHeaderRead()
    {
        return mFormatter->beginHeaderRead();
    }

    StreamResult FormattedBufferedStream::endHeaderRead()
    {
        return mFormatter->endHeaderRead();
    }

    void FormattedBufferedStream::beginHeaderWrite()
    {
        mFormatter->beginHeaderWrite();
    }

    void FormattedBufferedStream::endHeaderWrite()
    {
        mFormatter->endHeaderWrite();
    }

    message_streambuf &FormattedBufferedStream::buffer()
    {
        return static_cast<message_streambuf &>(mFormatter->stream().buffer());
    }

}
}