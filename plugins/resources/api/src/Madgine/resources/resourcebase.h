#pragma once

#include "Interfaces/filesystem/async.h"

namespace Engine {
namespace Resources {

    struct MADGINE_RESOURCES_EXPORT ResourceBase {
        ResourceBase(const std::string &name, Filesystem::Path path = {});

        ~ResourceBase() noexcept = default;

        void setPath(const Filesystem::Path &path);
        const Filesystem::Path &path() const;
        std::string_view extension();
        std::string_view name();

        Stream readAsStream(bool isBinary = false) const;
        std::string readAsText() const;
        std::vector<unsigned char> readAsBlob() const;

        void readAsyncImpl(Execution::VirtualReceiverBase<GenericResult, ByteBuffer> &rec) const;
        ASYNC_STUB(readAsync, readAsyncImpl, Execution::make_simple_virtual_sender<GenericResult, ByteBuffer>)

    private:        

        std::string mName;
        Filesystem::Path mPath;
    };

}
}