#pragma once

#include "Generic/stream.h"
#include "filequery.h"

namespace Engine {
namespace Filesystem {

    struct FileInfo {
        size_t mSize;
    };

    INTERFACES_EXPORT void setup(void *data = nullptr);

    INTERFACES_EXPORT FileQuery listFilesRecursive(const Path &path);
    INTERFACES_EXPORT FileQuery listFilesAndDirsRecursive(const Path &path);
    INTERFACES_EXPORT FileQuery listFiles(const Path &path);
    INTERFACES_EXPORT FileQuery listDirs(const Path &path);
    INTERFACES_EXPORT FileQuery listFilesAndDirs(const Path &path);
    INTERFACES_EXPORT Path executablePath();
    INTERFACES_EXPORT std::string executableName();
    INTERFACES_EXPORT Path appDataPath();
    INTERFACES_EXPORT Path tempPath();

    INTERFACES_EXPORT void makeNormalized(std::string &p);
    INTERFACES_EXPORT bool isValidPath(const std::string &p);

    INTERFACES_EXPORT Path getCwd();
    INTERFACES_EXPORT void setCwd(const Path &p);
    INTERFACES_EXPORT bool createDirectory(const Path &p);
    INTERFACES_EXPORT bool createDirectories(const Path &p);
    INTERFACES_EXPORT bool copyFile(const Path &file, const Path &target);
    INTERFACES_EXPORT bool exists(const Path &p);
    INTERFACES_EXPORT bool remove(const Path &p);
    INTERFACES_EXPORT bool isDir(const Path &p);
    INTERFACES_EXPORT bool isAbsolute(const Path &p);
    INTERFACES_EXPORT bool isSeparator(char c);
    INTERFACES_EXPORT bool isEqual(const Path &p1, const Path &p2);

    INTERFACES_EXPORT Stream openFileRead(const Path &p, bool isBinary = false);
    INTERFACES_EXPORT Stream openFileWrite(const Path &p, bool isBinary = false);
    INTERFACES_EXPORT FileInfo fileInfo(const Path &p);
}
}