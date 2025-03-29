#pragma once

#include "Meta/serialize/syncmanager.h"

#include "Meta/serialize/streams/syncstreamdata.h"

#include "Meta/serialize/formatter/safebinaryformatter.h"

#include "Meta/serialize/streams/buffered_streambuf.h"

#include "Generic/execution/algorithm.h"
#include "Generic/execution/execution.h"

#include "../../Generic/testreceiver.h"

#include "Meta/serialize/formats.h"

using namespace Engine::Serialize;

struct Buffer {
    std::array<char, 4096> mBuffer[2];
    uint64_t mWrittenCount[2] = { 0, 0 };
};

struct BufferedTestBuf : std::basic_streambuf<char> {
    BufferedTestBuf(Buffer &buffer, bool isMaster)
        : mBuffer(buffer)
        , mIsMaster(isMaster)
    {
    }

    /*virtual void handleError() override
    {        
    }*/

    virtual std::streamsize xsgetn(char *buffer, std::streamsize count) override
    {
        size_t index = mIsMaster ? 0 : 1;

        std::streamsize avail = mBuffer.mWrittenCount[index] - mReadOffset;
        std::streamsize readCount = std::min(count, avail);
        if (readCount == 0)
            return 0;
        std::memcpy(buffer, mBuffer.mBuffer[index].data() + mReadOffset, readCount);
        mReadOffset += readCount;
        return static_cast<int>(readCount);
    }

    virtual std::streamsize xsputn(const char *buffer, std::streamsize count) override
    {
        size_t index = mIsMaster ? 1 : 0;

        assert(mBuffer.mWrittenCount[index] + count <= mBuffer.mBuffer[index].size());
        std::memcpy(mBuffer.mBuffer[index].data() + mBuffer.mWrittenCount[index], buffer, count);
        mBuffer.mWrittenCount[index] += count;
        return static_cast<int>(count);
    }

    virtual std::streamsize showmanyc() override
    {
        size_t index = mIsMaster ? 0 : 1;
        return mBuffer.mWrittenCount[index] - mReadOffset;
    }

    Buffer &mBuffer;
    uint64_t mReadOffset = 0;
    bool mIsMaster;
};

struct TestBuf : std::basic_streambuf<char> {
    TestBuf(Buffer &buffer, bool isMaster = true)
        : mBuffer(buffer)
    {
        size_t index = isMaster ? 1 : 0;
        setp(mBuffer.mBuffer[index].data(), mBuffer.mBuffer[index].data());

        setg(mBuffer.mBuffer[1 - index].data(), mBuffer.mBuffer[1 - index].data(), mBuffer.mBuffer[1 - index].data() + mBuffer.mWrittenCount[1 - index]);
    }

    std::basic_streambuf<char>::int_type overflow(int c) override
    {
        if (c != EOF) {
            size_t index = 1;

            setp(pptr(), pptr() + 1);
            ++mBuffer.mWrittenCount[index];

            *pptr() = c;
            pbump(1);
            return c;
        }
        return traits_type::eof();
    }

    std::basic_streambuf<char>::int_type underflow() override
    {
        size_t index = 1;
        if (mBuffer.mWrittenCount[index] > (egptr() - eback())) {
            setg(eback(), gptr(), eback() + mBuffer.mWrittenCount[index]);
            std::basic_streambuf<char>::int_type c = *gptr();
            //gbump(1);
            return c;
        }
        return traits_type::eof();
    }

    Buffer &mBuffer;
};

struct TestManager : SyncManager {
    TestManager(const char *id)
        : SyncManager("Test-Manager_"s + id)
    {
    }

    void setSlaveBuffer(
        TestReceiver<Engine::Serialize::SyncManagerResult> &receiver, Buffer &buffer, Engine::Serialize::Format format = Engine::Serialize::Formats::safebinary)
    {
        std::unique_ptr<buffered_streambuf> buf = std::make_unique<buffered_streambuf>(std::make_unique<BufferedTestBuf>(buffer, false));
        Engine::Execution::detach(
            setSlaveStream(format, std::move(buf), 1s, std::make_unique<SyncStreamData>(*this, 0))
            | Engine::Execution::then_receiver(receiver));
        receiveMessages(-1, 1s);
    }

    SyncManagerResult setMasterBuffer(
        Buffer &buffer, Engine::Serialize::Format format = Engine::Serialize::Formats::safebinary)
    {
        std::unique_ptr<buffered_streambuf> buf = std::make_unique<buffered_streambuf>(std::make_unique<BufferedTestBuf>(buffer, true));
        return addMasterStream(format, std::move(buf), std::make_unique<SyncStreamData>(*this, 1));
    }

    using SyncManager::getMasterStream;
    using SyncManager::getSlaveStream;
};

#define HANDLE_MGR_RESULT(mgr, ...) ASSERT_EQ(__VA_ARGS__, SyncManagerResult::SUCCESS)
#define HANDLE_STREAM_RESULT(...)                            \
    {                                                        \
        StreamResult result = __VA_ARGS__;                   \
        ASSERT_EQ(result.mState, StreamState::OK) << result; \
    }
#define ASSERT_MESSAGEFUTURE_EQ(f, ...)   \
    {                                     \
        ASSERT_TRUE(f.mValue);            \
        ASSERT_EQ(f.mValue, __VA_ARGS__); \
    }
#define HANDLE_MGR_RECEIVER(...)                  \
    {                                             \
        TestReceiver<SyncManagerResult> receiver; \
        __VA_ARGS__;                              \
        ASSERT_TRUE(receiver.mFinished);          \
        ASSERT_TRUE(receiver.mHasValue);          \
    }