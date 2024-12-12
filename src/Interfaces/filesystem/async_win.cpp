#include "../interfaceslib.h"

#if WINDOWS

#    include "async.h"

#    include "Generic/align.h"

#    include "../helpers/win_ptrs.h"

#    define NOMINMAX
#    include <Windows.h>

namespace Engine {
namespace Filesystem {

    static std::mutex sIOMutex;

    static HANDLE sCompletionPort = NULL;

    static std::atomic_flag sEnqueuesAllowed = ATOMIC_FLAG_INIT;

    struct AsyncFileReadAuxiliaryData {
        UniqueHandle mHandle;
        OVERLAPPED mOverlapped = {};
        AsyncFileReadAuxiliaryData *mNext = nullptr;
    };

    static std::atomic<AsyncFileReadAuxiliaryData *> sPending = nullptr;
    static std::atomic<size_t> sPendingCount = 0;

    void setup_async()
    {
        sCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
        assert(sCompletionPort);
        bool test = sEnqueuesAllowed.test_and_set();
        assert(!test);
    }

    void checkAsyncIOCompletion()
    {
        LPOVERLAPPED overlapped = NULL;
        ULONG_PTR key;
        DWORD bytesTransferred;
        
        while (true) {
            bool result = GetQueuedCompletionStatus(sCompletionPort,
                &bytesTransferred,
                &key,
                &overlapped,
                0);

            if (!result && !overlapped)
                break;

            AsyncFileReadAuxiliaryData *data = sPending.load();
            if (&data->mOverlapped != overlapped || !sPending.compare_exchange_strong(data, data->mNext)) {
                bool found = false;
                while (data->mNext) {
                    if (&data->mNext->mOverlapped == overlapped) {
                        data->mNext = data->mNext->mNext;
                        found = true;
                        break;
                    }
                    data = data->mNext;
                }
                assert(found);
            }
            --sPendingCount;

            AsyncFileReadState *state = reinterpret_cast<AsyncFileReadState *>(key);
            if (result) {
                assert(bytesTransferred == state->mBuffer.mSize);

                state->set_value(std::move(state->mBuffer));
            } else {
                auto error = GetLastError();
                if (error == ERROR_OPERATION_ABORTED) {
                    state->set_done();
                } else {
                    state->set_error(GenericResult::UNKNOWN_ERROR);
                }
            }
        }
    }

    void cancelAllAsyncIO()
    {
        sEnqueuesAllowed.clear();
        AsyncFileReadAuxiliaryData *data = sPending.load();
        while (data) {
            bool result = CancelIo(data->mHandle);
            assert(result);
            data = data->mNext;
        }        
    }

    size_t pendingIOOperationCount()
    {
        return sPendingCount;
    }

    AsyncFileReadState::AsyncFileReadState(Path path)
        : mPath(std::move(path))
        , mData(std::make_unique<AsyncFileReadAuxiliaryData>())
    {
    }

    AsyncFileReadState::~AsyncFileReadState() = default;

    static void sFreeBuffer(void *ptr)
    {
        bool result = VirtualFree(ptr, 0, MEM_RELEASE);
        assert(result);
    }

    void AsyncFileReadState::start()
    {
        ++sPendingCount;
        if (!sEnqueuesAllowed.test()) {
            --sPendingCount;
            set_done();
            return;
        }

        HANDLE handle = CreateFile(mPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
        assert(handle != INVALID_HANDLE_VALUE);
        mData->mHandle = handle;

        LARGE_INTEGER fileSize;

        bool result = GetFileSizeEx(
            handle,
            &fileSize);
        assert(result);

        size_t size = fileSize.QuadPart;

        std::unique_ptr<void, Functor<sFreeBuffer>> buffer { VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE) };

        assert(buffer);

        assert(sCompletionPort);
        HANDLE ioPort = CreateIoCompletionPort(handle, sCompletionPort, reinterpret_cast<uintptr_t>(this), 0);
        assert(ioPort == sCompletionPort);

        while (!sPending.compare_exchange_weak(mData->mNext, mData.get()))
            ;

        mData->mOverlapped.Offset = 0;
        mData->mOverlapped.OffsetHigh = 0;
        result = ReadFile(handle, buffer.get(), alignTo(size, 512), nullptr, &mData->mOverlapped);

        assert(!result);
        
        assert(GetLastError() == ERROR_IO_PENDING);

        mBuffer = {
            std::move(buffer),
            size
        };
    }

}
}

#endif