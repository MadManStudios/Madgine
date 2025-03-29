#pragma once

#include "Meta/serialize/serializemanager.h"
#include "Interfaces/filesystem/path.h"

namespace Engine {
namespace Filesystem {
    struct MADGINE_FILESYSTEM_SERIALIZE_EXPORT FileManager : Serialize::SerializeManager {    
        FileManager(const std::string &name);
        FileManager(const FileManager &) = delete;
        FileManager(FileManager &&) noexcept = default; 
        virtual ~FileManager() = default;

        void operator=(const FileManager &) = delete;

        Serialize::FormattedSerializeStream openRead(const Path &path, Serialize::Format format);
        Serialize::FormattedSerializeStream openWrite(const Path &path, Serialize::Format format);
    };
}
}
