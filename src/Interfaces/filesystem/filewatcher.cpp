#include "../interfaceslib.h"

#include "filewatcher.h"

namespace Engine {
namespace Filesystem {

    std::vector<FileEvent> FileWatcher::fetchChangesReduced()
    {
        static Filesystem::FileEventType::EnumType invalid = static_cast<Filesystem::FileEventType::EnumType>(Filesystem::FileEventType::MAX);

        std::vector<FileEvent> events = fetchChanges();
        for (auto it = events.rbegin(); it != events.rend(); ++it) {
            switch (it->mType) {
            case Filesystem::FileEventType::FILE_DELETED:
                for (auto it2 = std::next(it); it2 != events.rend(); ++it2) {
                    bool abort = false;
                    if (it2->mPath == it->mPath) {
                        switch (it2->mType) {
                        case Filesystem::FileEventType::FILE_CREATED:
                            abort = true;
                            it2->mType = invalid;
                            it->mType = invalid;
                            break;
                        case Filesystem::FileEventType::FILE_MODIFIED:
                            it2->mType = invalid;
                            break;
                        case Filesystem::FileEventType::FILE_RENAMED:
                            it2->mType = Filesystem::FileEventType::FILE_DELETED;
                            it->mType = invalid;
                            it2->mPath = it2->mOldPath;
                            abort = true;
                            break;
                        default:
                            break;
                        }
                    }
                    if (abort)
                        break;
                }
                break;
            case Filesystem::FileEventType::FILE_RENAMED:
                for (auto it2 = std::next(it); it2 != events.rend(); ++it2) {
                    bool abort = false;
                    if (it2->mPath == it->mPath) {
                        switch (it2->mType) {
                        case Filesystem::FileEventType::FILE_DELETED:
                            abort = true;
                            it2->mType = Filesystem::FileEventType::FILE_DELETED;
                            it2->mPath = it->mOldPath;
                            it->mType = Filesystem::FileEventType::FILE_MODIFIED;
                            break;
                        default:
                            break;
                        }
                    }
                    if (abort)
                        break;
                }
                break;
            default:
                break;
            }
        }

        std::erase_if(events, [](const Filesystem::FileEvent &event) { return event.mType == invalid; });

        return events;
    }

}
}