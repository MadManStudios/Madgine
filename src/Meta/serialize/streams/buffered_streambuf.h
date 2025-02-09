#pragma once

#include "message_streambuf.h"


namespace Engine {
namespace Serialize {

    struct BufferedMessageHeader {
        uint64_t mMsgSize;
        MessageId mMessageId;
    };

    struct BufferedSendMessage {
        std::vector<char> mData;
    };

    struct META_EXPORT buffered_streambuf : message_streambuf {
    public:
        buffered_streambuf(std::unique_ptr<std::basic_streambuf<char>> buffer);
        buffered_streambuf(const buffered_streambuf &) = delete;
        buffered_streambuf(buffered_streambuf &&) = delete;

        virtual ~buffered_streambuf();

        void beginMessageWriteImpl() override;
        MessageId endMessageWriteImpl() override;

        MessageId beginMessageReadImpl() override;
        std::streamsize endMessageReadImpl() override;

        std::basic_streambuf<char> &buffer() const;

    protected:
        pos_type seekoff(off_type off, std::ios_base::seekdir dir,
            std::ios_base::openmode mode = std::ios_base::in) override;
        pos_type seekpos(pos_type pos,
            std::ios_base::openmode mode = std::ios_base::in) override;

        int_type overflow(int c = EOF) override;

        void extend();
        StreamResult receiveMessages() override;
        StreamResult sendMessages() override;

    private:
        //read
        uint64_t mBytesToRead;
        BufferedMessageHeader mReceiveMessageHeader;
        std::vector<char> mRecBuffer;

        //write
        std::vector<char> mSendBuffer;

        struct BufferedMessage {
            BufferedMessageHeader mHeader;
            char mData[0];
        };

        std::queue<BufferedSendMessage> mBufferedSendMsgs;

        std::unique_ptr<std::basic_streambuf<char>> mBuffer;

        MessageId mRunningMessageId = 0;
    };
}
}
