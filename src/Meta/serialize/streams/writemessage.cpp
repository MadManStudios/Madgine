#include "../../metalib.h"

#include "writemessage.h"

#include "formattedmessagestream.h"

#include "message_streambuf.h"

namespace Engine {
namespace Serialize {

	WriteMessage::WriteMessage(FormattedMessageStream *stream, ParticipantId requester, MessageId requestId, GenericMessageReceiver receiver)
        : mStream(stream)
        , mRequester(requester)
        , mRequestId(requestId)
        , mReceiver(std::move(receiver))
    {
        stream->buffer().beginMessageWrite();
        stream->mFormatter->beginMessageWrite();
    }

    WriteMessage::WriteMessage(WriteMessage &&other)
        : mStream(std::exchange(other.mStream, nullptr))
        , mRequester(other.mRequester)
        , mRequestId(other.mRequestId)
        , mReceiver(std::move(other.mReceiver))
    {
    }

    WriteMessage::~WriteMessage()
    {
        if (mStream) {
            mStream->mFormatter->endMessageWrite();
            mStream->buffer().endMessageWrite(mRequester, mRequestId, std::move(mReceiver));
        }
    }

    WriteMessage &WriteMessage::operator=(WriteMessage &&other)
    {
        assert(!mStream);
        mStream = std::exchange(other.mStream, nullptr);
        mRequester = other.mRequester;
        mRequestId = other.mRequestId;
        mReceiver = std::move(other.mReceiver);
        return *this;
    }

    SerializeStream &WriteMessage::stream()
    {
        return mStream->stream();
    }

    WriteMessage::operator FormattedMessageStream &() const
    {
        return *mStream;
    }


}
}