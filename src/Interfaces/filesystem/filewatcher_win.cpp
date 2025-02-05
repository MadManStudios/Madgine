#include "../interfaceslib.h"

#if WINDOWS

#    include "fsapi.h"

#    include "filewatcher.h"

#    include "Generic/coroutines/generator.h"

#    include "../helpers/win_ptrs.h"

#    define NOMINMAX
#    include <Windows.h>

namespace Engine {
namespace Filesystem {
#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)

    PFILE_NOTIFY_INFORMATION bump(PFILE_NOTIFY_INFORMATION p)
    {
        return p->NextEntryOffset == 0 ? nullptr : reinterpret_cast<PFILE_NOTIFY_INFORMATION>(reinterpret_cast<char *>(p) + p->NextEntryOffset);
    }

    Generator<std::vector<FileEvent>> fileWatch(const Path &path)
    {

        std::vector<FileEvent> result;

        UniqueHandle dir = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
        UniqueHandle event = CreateEvent(NULL, true, false, path.c_str());
        assert(event);

        OVERLAPPED overlapped;
        overlapped.hEvent = event;

        FILE_NOTIFY_INFORMATION info[50];

        while (true) {

            ResetEvent(event);

            bool b = ReadDirectoryChangesW(dir, info, sizeof(info), true, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME, NULL, &overlapped, NULL);
            if (!b) {
                LOG_ERROR("Error: " << GetLastError());
                std::terminate();
            }
            DWORD bytes;
            while (!GetOverlappedResult(dir, &overlapped, &bytes, false)) {
                auto error = GetLastError();
                switch (error) {
                case ERROR_IO_INCOMPLETE:
                    co_yield result;
                    break;
                default:
                    LOG_ERROR("Error: " << error);
                    std::terminate();
                }
            }
            assert(bytes);
            PFILE_NOTIFY_INFORMATION pInfo = info;
            while (pInfo) {
                FileEvent &event = result.emplace_back();

                std::wstring ws(pInfo->FileName, pInfo->FileNameLength / 2);
                event.mPath = path / std::string { ws.begin(), ws.end() };

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
                    event.mType = FileEventType::FILE_RENAMED;
                    event.mOldPath = event.mPath;
                    pInfo = bump(pInfo);
                    if (!pInfo)
                        std::terminate();
                    if (pInfo->Action != FILE_ACTION_RENAMED_NEW_NAME)
                        std::terminate();
                    ws = { pInfo->FileName, pInfo->FileNameLength / 2 };
                    event.mPath = path / std::string { ws.begin(), ws.end() };
                    break;
                case FILE_ACTION_RENAMED_NEW_NAME:
                    throw 0;
                default:
                    std::terminate();
                }

                pInfo = bump(pInfo);
            }
            co_yield result;
            result.clear();
        }
    }
#    endif

    FileWatcher::FileWatcher()
    {
    }

    FileWatcher::~FileWatcher()
    {
        clear();
    }

    void FileWatcher::addWatch(const Path &path)
    {
#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
        assert(isDir(path));
        auto pib = mWatches.try_emplace(path);
        if (pib.second) {
            pib.first->second.setupAs<Generator<std::vector<FileEvent>>>() = fileWatch(pib.first->first);
        }
#    endif
    }

    void FileWatcher::removeWatch(const Path &path)
    {
#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
        auto it = mWatches.find(path);
        if (it != mWatches.end()) {
            // cause destroy
            it->second.release<Generator<std::vector<FileEvent>>>();
            mWatches.erase(it);
        }
#    endif
    }

    void FileWatcher::clear()
    {
#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
        for (std::pair<const Path, UniqueOpaquePtr> &handle : mWatches) {
            // cause destroy
            handle.second.release<Generator<std::vector<FileEvent>>>();            
        }
        mWatches.clear();
#    endif
    }

    std::vector<FileEvent> FileWatcher::fetchChanges()
    {
        std::vector<FileEvent> result;

#    if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)
        for (std::pair<const Path, UniqueOpaquePtr> &watch : mWatches) {
            Generator<std::vector<FileEvent>> &gen = watch.second.as<Generator<std::vector<FileEvent>>>();
            gen.next();
            const std::vector<FileEvent> &events = gen.get();
            std::ranges::copy(events, std::back_inserter(result));
        }
#    endif

        return result;
    }

}
}

#endif