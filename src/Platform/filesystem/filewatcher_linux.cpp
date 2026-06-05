#include "../platformlib.h"

#if LINUX

#    include "filewatcher.h"

namespace Engine {
namespace Platform {
    namespace Filesystem {

        FileWatcher::FileWatcher()
        {
        }

        FileWatcher::~FileWatcher()
        {
            /*for (const std::pair<const Path, uintptr_t> &handle : mWatches) {
                CloseHandle((HANDLE)handle.second);
            }*/
        }

        void FileWatcher::addWatch(const Path &path)
        {
            /*assert(isDir(path));
            auto pib = mWatches.try_emplace(path, 0);
            if (pib.second) {
                HANDLE dir = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
                if (!dir)
                    throw 0;
                pib.first->second = (uintptr_t)dir;
            }*/
        }

        void FileWatcher::removeWatch(const Path &path)
        {
            /*auto it = mWatches.find(path);
            if (it != mWatches.end()) {
                CloseHandle((HANDLE)it->second);
                mWatches.erase(it);
            }*/
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
            std::vector<FileEvent> result;

            /*for (const std::pair<const Path, uintptr_t> &watch : mWatches) {
                FILE_NOTIFY_INFORMATION info[10];
                DWORD bytes;
                bool b = ReadDirectoryChangesW((HANDLE)watch.second, info, sizeof(info), true, FILE_NOTIFY_CHANGE_LAST_WRITE, &bytes, NULL, NULL);
                if (!b) {
                    LOG_ERROR("Error: " << GetLastError());
                    std::terminate();
                }
                if (bytes) {
                    PFILE_NOTIFY_INFORMATION pInfo = info;
                    while (pInfo) {
                        FileEvent &event = result.emplace_back();

                        std::wstring ws(pInfo->FileName, pInfo->FileNameLength / 2);
                        event.mPath = watch.first / std::string { ws.begin(), ws.end() };

                        switch (pInfo->Action) {
                        case FILE_ACTION_ADDED:
                            event.mType = FileEventType::FILE_CREATED;
                            break;
                        case FILE_ACTION_REMOVED:
                            event.mType = FileEventType::FILE_DELETED;
                            break;
                        case FILE_ACTION_MODIFIED:
                            event.mType = FileEventType::FILE_MODIFIED;
                            break;
                        case FILE_ACTION_RENAMED_OLD_NAME:
                            event.mOldPath = event.mPath;
                            pInfo = bump(pInfo);
                            if (!pInfo)
                                std::terminate();
                            if (pInfo->Action != FILE_ACTION_RENAMED_NEW_NAME)
                                std::terminate();
                            ws = { pInfo->FileName, pInfo->FileNameLength / 2 };
                            event.mPath = watch.first / std::string { ws.begin(), ws.end() };
                            break;
                        case FILE_ACTION_RENAMED_NEW_NAME:
                            throw 0;
                        default:
                            std::terminate();
                        }

                        pInfo = bump(pInfo);
                    }
                }
            }*/
            return result;
        }

    }
}
}

#endif