#pragma once

#include "Platform/filesystem/async.h"

#include "Generic/execution/sender.h"

namespace Engine {
namespace Resources {

    struct MADGINE_RESOURCES_EXPORT ResourceBase {
        ResourceBase(const std::string &name, Platform::Filesystem::Path path = {});

        ~ResourceBase() noexcept = default;

        void setPath(const Platform::Filesystem::Path &path);
        const Platform::Filesystem::Path &path() const;
        std::string_view extension();
        std::string_view name();

        std::string plugin() const;

        Stream readAsStream(bool isBinary = false) const;
        std::string readAsText() const;
        std::vector<unsigned char> readAsBlob() const;

        Execution::Sender<GenericResult, Memory::ByteBuffer> readAsync() const;

    private:
        std::string mName;
        Platform::Filesystem::Path mPath;
    };

}
}