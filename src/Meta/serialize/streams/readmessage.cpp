#include "../../metalib.h"

#include "readmessage.h"

#include "streamresult.h"

#include "formatter.h"

#include "message_streambuf.h"

namespace Engine {
namespace Serialize {

	
    ReadMessage::~ReadMessage()
    {
    }

    ReadMessage &ReadMessage::operator=(ReadMessage &&other)
    {
        assert(!mId);
        mId = std::exchange(other.mId, 0);
        mFormatter = std::move(other.mFormatter);
        return *this;
    }

    StreamResult ReadMessage::end()
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

    ReadMessage::operator bool() const
    {
        return mId != 0;
    }


}
}