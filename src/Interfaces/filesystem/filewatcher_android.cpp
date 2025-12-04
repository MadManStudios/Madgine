#include "../interfaceslib.h"

#if ANDROID

#    include "filewatcher.h"

namespace Engine {
namespace Filesystem {

    FileWatcher::FileWatcher()
    {
    }

    FileWatcher::~FileWatcher()
    {
    }

    void FileWatcher::addWatch(const Path &path)
    {
    }

    void FileWatcher::removeWatch(const Path &path)
    {
    }

    void FileWatcher::clear()
    {
        /*for (const std::pair<const Path, uintptr_t> &handle : mWatches) {
            Generator<std::vector<FileEvent>>::fromAddress(reinterpret_cast<void *>(handle.second)).reset();
        }
        mWatches.clear();*/
    }

    std::vector<FileEvent> FileWatcher::fetchChanges()
    {
        return {};
    }

}
}

#endif