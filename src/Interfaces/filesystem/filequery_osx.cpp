#include "../interfaceslib.h"

#if OSX

#    include "fsapi.h"

#    include <dirent.h>
#    include <sys/stat.h>
#    include <unistd.h>

namespace Engine {
namespace Filesystem {

    struct FileQueryState {
        dirent *mData;
        Path mPath;
    };

    static bool skipSymbolic(void *handle, FileQueryState &data)
    {
        while (strcmp(data.mData->d_name, ".") == 0 || strcmp(data.mData->d_name, "..") == 0) {
            data.mData = readdir((DIR *)handle);
            if (!data.mData)
                return false;
        }

        return true;
    }

    FileQueryHandle::FileQueryHandle(Path p, FileQueryState &data)
        : mPath(std::move(p))
        , mHandle(opendir(mPath.c_str()))
    {
        if (mHandle) {
            if (!advance(data)) {
                close();
            }
        }
    }

    void FileQueryHandle::close()
    {
        if (mHandle) {
            closedir((DIR *)mHandle);
            mHandle = nullptr;
        }
    }

    bool FileQueryHandle::advance(FileQueryState &data)
    {
        data.mData = readdir((DIR *)mHandle);
        data.mPath = mPath;
        if (!data.mData)
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
        return isDir(data.mPath / data.mData->d_name);
    }

    const char *filename(const FileQueryState &data)
    {
        return data.mData->d_name;
    }

}
}

#endif
