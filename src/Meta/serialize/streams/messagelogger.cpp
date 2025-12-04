#include "../../metalib.h"

#include "messagelogger.h"

#if ENABLE_MESSAGE_LOGGING

namespace Engine {
namespace Serialize {

    static MessageLogger sSingleton;

    void MessageLogger::registerStream(buffered_streambuf *buffer)
    {
        std::unique_lock lock { sSingleton.mMutex };
        StreamInfo &stream = sSingleton.mStreams[buffer];
        stream.mThread = std::this_thread::get_id();
    }

    void MessageLogger::unregisterStream(buffered_streambuf *buffer)
    {
        std::unique_lock lock { sSingleton.mMutex };
        sSingleton.mStreams.erase(buffer);
    }

    MessageLogger::MessageInfo &MessageLogger::log(buffered_streambuf *buffer, BufferedSendMessage msg)
    {
        std::unique_lock lock { sSingleton.mMutex };
        StreamInfo &stream = sSingleton.mStreams.at(buffer);
        BufferedMessageHeader header = *reinterpret_cast<BufferedMessageHeader *>(msg.mData.data());
        msg.mData.erase(msg.mData.begin(), msg.mData.begin() + sizeof(BufferedMessageHeader));
        MessageLogger::MessageInfo &info = stream.mMessages.emplace_back(std::move(header), std::move(msg.mData));
        info.mDir = OUTGOING;
        stream.mThread = std::this_thread::get_id();
        size_t id = stream.mMessages.size();
        return info;
    }

    MessageLogger::MessageInfo &MessageLogger::log(buffered_streambuf *buffer, BufferedMessageHeader header, std::vector<char> data)
    {
        std::unique_lock lock { sSingleton.mMutex };
        StreamInfo &stream = sSingleton.mStreams.at(buffer);
        MessageLogger::MessageInfo &info = stream.mMessages.emplace_back(std::move(header), std::move(data));
        info.mDir = INCOMING;
        stream.mThread = std::this_thread::get_id();
        size_t id = stream.mMessages.size();
        return info;
    }

    MessageLogger &MessageLogger::getSingleton()
    {
        return sSingleton;
    }

    MessageLogger::MessageInfo::MessageInfo(BufferedMessageHeader header, std::vector<char> data)
        : mHeader(std::move(header))
        , mData(std::move(data))
    {
    }

}
}

#endif