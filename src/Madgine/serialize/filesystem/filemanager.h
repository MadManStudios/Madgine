#pragma once

#include "Platform/filesystem/path.h"

#include "Meta/serialize/serializemanager.h"

namespace Engine {
namespace Serialize {
    struct MADGINE_FILESYSTEM_SERIALIZE_EXPORT FileManager : SerializeManager {
        FileManager(const std::string &name);
        FileManager(const FileManager &) = delete;
        FileManager(FileManager &&) noexcept = default;
        virtual ~FileManager() = default;

        void operator=(const FileManager &) = delete;

        Serialize::FormattedSerializeStream openRead(const Platform::Filesystem::Path &path, Format format);
        Serialize::FormattedSerializeStream openWrite(const Platform::Filesystem::Path &path, Format format);
    };
}
}
