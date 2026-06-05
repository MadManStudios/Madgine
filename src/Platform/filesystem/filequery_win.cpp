#include "../platformlib.h"

#if WINDOWS

#    include "filequery.h"
#    include "path.h"

#    define NOMINMAX
#    include <Windows.h>
#    include <cctype>
#    include <direct.h>

namespace Engine {
namespace Platform {
    namespace Filesystem {

        struct FileQueryState {
            WIN32_FIND_DATA mData;
        };

        static bool skipSymbolic(void *handle, FileQueryState &data)
        {
            while (strcmp(data.mData.cFileName, ".") == 0 || strcmp(data.mData.cFileName, "..") == 0)
                if (!FindNextFile(handle, &data.mData))
                    return false;
            return true;
        }

        FileQueryHandle::FileQueryHandle(Path p, FileQueryState &data)
            : mPath(std::move(p))
            , mHandle(FindFirstFile((mPath.str() + "/*").c_str(), &data.mData))
        {
            if (mHandle == INVALID_HANDLE_VALUE)
                mHandle = nullptr;
            else if (!skipSymbolic(mHandle, data))
                close();
        }

        void FileQueryHandle::close()
        {
            if (mHandle) {
                FindClose(mHandle);
                mHandle = nullptr;
            }
        }

        bool FileQueryHandle::advance(FileQueryState &data)
        {
            if (!FindNextFile(mHandle, &data.mData))
                return false;
            return skipSymbolic(mHandle, data);
        }

        FileQueryState *createQueryState()
        {
            return new FileQueryState;
        }

        void destroyQueryState(FileQueryState *state)
        {
            delete state;
        }

        bool isDir(const FileQueryState &data)
        {
            return (data.mData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        }

        const char *filename(const FileQueryState &data)
        {
            return data.mData.cFileName;
        }

    }
}
}

#endif