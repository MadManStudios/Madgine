#include "../../metalib.h"
#include "message_streambuf.h"
#include "streamresult.h"

namespace Engine {
namespace Serialize {

    void message_streambuf::beginMessageWrite()
    {
        assert(!pptr());
        beginMessageWriteImpl();
        assert(pptr());
    }

    void message_streambuf::endMessageWrite(ParticipantId requester, MessageId requestId, GenericMessageReceiver receiver)
    {
        assert(pptr());
        MessageId id = endMessageWriteImpl();
        setp(nullptr, nullptr);
        if (requester || receiver)
            mPendingRequests.push_back({ id, requester, requestId, std::move(receiver) });
    }

    MessageId message_streambuf::beginMessageRead()
    {
        assert(!egptr());
        MessageId result = beginMessageReadImpl();
        assert(result == 0 || egptr());
        return result;
    }

    std::streamsize message_streambuf::endMessageRead()
    {
        std::streamsize remainder = endMessageReadImpl();
        setg(nullptr, nullptr, nullptr);
        return remainder;
    }

    PendingRequest message_streambuf::getRequest(MessageId id)
    {
        auto it = std::ranges::find(mPendingRequests, id, &PendingRequest::mId);
        if (it == mPendingRequests.end())
            return { id };
        else {
            PendingRequest result = std::move(*it);
            mPendingRequests.erase(it);
            return result;
        }
    }

    std::streamsize message_streambuf::endMessageReadImpl() {
        std::streamsize result = showmanyc();
        setg(nullptr, nullptr, nullptr);
        return result;
    }

}
}
