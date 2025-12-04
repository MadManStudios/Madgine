#pragma once

#if ENABLE_MESSAGE_LOGGING

#    include "buffered_streambuf.h"

namespace Engine {
namespace Serialize {

    struct META_EXPORT MessageLogger {

        enum Direction {
            INCOMING,
            OUTGOING
        };

        struct MessageInfo {
            MessageInfo(BufferedMessageHeader header, std::vector<char> data);

            Direction mDir;
            BufferedMessageHeader mHeader;
            std::vector<char> mData;
        };

        struct StreamInfo {
            std::vector<MessageInfo> mMessages;

            std::thread::id mThread;
        };

        static void registerStream(buffered_streambuf *buffer);
        static void unregisterStream(buffered_streambuf *buffer);
        static MessageInfo &log(buffered_streambuf *buffer, BufferedSendMessage msg);
        static MessageInfo &log(buffered_streambuf *buffer, BufferedMessageHeader header, std::vector<char> data);

        static MessageLogger &getSingleton();

        std::map<buffered_streambuf *, StreamInfo> mStreams;
        std::mutex mMutex;
    };

}
}

#endif