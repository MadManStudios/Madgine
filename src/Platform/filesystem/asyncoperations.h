#pragma once

#include "Generic/bytebuffer.h"
#include "Generic/execution/concepts.h"
#include "Generic/execution/virtualstate.h"
#include "Generic/functor.h"
#include "Generic/genericresult.h"

#include "path.h"

namespace Engine {
namespace Platform {
    namespace Filesystem {

        struct AsyncFileReadAuxiliaryData;

        struct PLATFORM_EXPORT AsyncFileReadState : Execution::VirtualReceiverBase<GenericResult, Memory::ByteBuffer> {

            AsyncFileReadState(Path path);
            AsyncFileReadState(const AsyncFileReadState &) = delete;
            AsyncFileReadState(AsyncFileReadState &&) = delete;
            ~AsyncFileReadState();

            AsyncFileReadState &operator=(const AsyncFileReadState &) = delete;
            AsyncFileReadState &operator=(AsyncFileReadState &&) = delete;

            void start();
            void stop();

            Path mPath;

            std::unique_ptr<AsyncFileReadAuxiliaryData> mData;

            Memory::ByteBuffer mBuffer;
        };

    }
}
}