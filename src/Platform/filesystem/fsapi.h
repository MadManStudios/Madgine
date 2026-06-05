#pragma once

#include "Generic/stream.h"

#include "filequery.h"

namespace Engine {
namespace Platform {
    namespace Filesystem {

        struct FileInfo {
            size_t mSize;
            std::chrono::file_clock::time_point mLastModified;
        };

        PLATFORM_EXPORT void setup(void *data = nullptr);

        PLATFORM_EXPORT FileQuery listFilesRecursive(const Path &path);
        PLATFORM_EXPORT FileQuery listFilesAndDirsRecursive(const Path &path);
        PLATFORM_EXPORT FileQuery listFiles(const Path &path);
        PLATFORM_EXPORT FileQuery listDirs(const Path &path);
        PLATFORM_EXPORT FileQuery listFilesAndDirs(const Path &path);
        PLATFORM_EXPORT Path executablePath();
        PLATFORM_EXPORT std::string executableName();
        PLATFORM_EXPORT Path appDataPath();
        PLATFORM_EXPORT Path tempPath();
        PLATFORM_EXPORT Path shippingPath();

        PLATFORM_EXPORT void makeNormalized(std::string &p);
        PLATFORM_EXPORT bool isValidPath(const std::string &p);

        PLATFORM_EXPORT Path getCwd();
        PLATFORM_EXPORT void setCwd(const Path &p);
        PLATFORM_EXPORT bool createDirectory(const Path &p);
        PLATFORM_EXPORT bool createDirectories(const Path &p);
        PLATFORM_EXPORT bool copyFile(const Path &file, const Path &target);
        PLATFORM_EXPORT bool exists(const Path &p);
        PLATFORM_EXPORT bool remove(const Path &p);
        PLATFORM_EXPORT bool isDir(const Path &p);
        PLATFORM_EXPORT bool isAbsolute(const Path &p);
        PLATFORM_EXPORT bool isSeparator(char c);
        PLATFORM_EXPORT bool isEqual(const Path &p1, const Path &p2);

        PLATFORM_EXPORT Stream openFileRead(const Path &p, bool isBinary = false);
        PLATFORM_EXPORT Stream openFileWrite(const Path &p, bool isBinary = false);
        PLATFORM_EXPORT FileInfo fileInfo(const Path &p);
    }
}
}