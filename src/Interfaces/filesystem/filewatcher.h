#pragma once

#include "Generic/enum.h"

#include "path.h"

#include "Generic/opaqueptr.h"

namespace Engine {
namespace Filesystem {

    ENUM(FileEventType,
        FILE_MODIFIED,
        FILE_CREATED,
        FILE_RENAMED,
        FILE_DELETED)

    struct FileEvent {
        FileEventType mType;
        Path mPath;
        Path mOldPath;
    };

    struct INTERFACES_EXPORT FileWatcher {

        FileWatcher();
        FileWatcher(const FileWatcher &) = delete;
        FileWatcher(FileWatcher &&) = delete;
        ~FileWatcher();

        void addWatch(const Path &path);
        void removeWatch(const Path &path);

        void clear();

        std::vector<FileEvent> fetchChanges();
        std::vector<FileEvent> fetchChangesReduced();

    private:
        std::map<Path, UniqueOpaquePtr> mWatches;
    };

}
}