#pragma once

#include "Generic/bytebuffer.h"
#include "Generic/functor.h"
#include "Generic/genericresult.h"
#include "Generic/execution/virtualstate.h"
#include "Generic/execution/concepts.h"
#include "path.h"

namespace Engine {
namespace Filesystem {

    struct AsyncFileReadAuxiliaryData;

    struct INTERFACES_EXPORT AsyncFileReadState : Execution::VirtualReceiverBase<GenericResult, ByteBuffer> {
        
        AsyncFileReadState(Path path);
        AsyncFileReadState(const AsyncFileReadState &) = delete;
        AsyncFileReadState(AsyncFileReadState &&) = delete;
        ~AsyncFileReadState();

        AsyncFileReadState &operator=(const AsyncFileReadState &) = delete;
        AsyncFileReadState &operator=(AsyncFileReadState &&) = delete;

        void start();

        Path mPath;

        std::unique_ptr<AsyncFileReadAuxiliaryData> mData;

        ByteBuffer mBuffer;
    };

}
}