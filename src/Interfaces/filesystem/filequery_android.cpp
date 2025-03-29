#include "../interfaceslib.h"

#if ANDROID

#    include "fsapi.h"

#    include <dirent.h>
#    include <sys/stat.h>
#    include <unistd.h>

#    include <android/asset_manager.h>

namespace Engine {
namespace Filesystem {

    extern AAssetManager *sAssetManager;

    static const char sAssetPrefix[] = "assets:";

    static bool isAssetPath(const Path &p)
    {
        return StringUtil::startsWith(p.str(), sAssetPrefix);
    }

    static const char *assetDir(const Path &p)
    {
        assert(isAssetPath(p));
        const char *dir = p.c_str() + sizeof(sAssetPrefix) - 1;
        if (isSeparator(*dir))
            ++dir;
        return dir;
    }

    struct FileQueryState {
        union {
            dirent *mData;
            const char *mAssetData;
        };
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
        , mHandle(isAssetPath(mPath) ? (void *)AAssetManager_openDir(sAssetManager, assetDir(mPath)) : (void *)opendir(mPath.c_str()))
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
            if (isAssetPath(mPath))
                AAssetDir_close((AAssetDir *)mHandle);
            else
                closedir((DIR *)mHandle);
            mHandle = nullptr;
        }
    }

    bool FileQueryHandle::advance(FileQueryState &data)
    {
        data.mPath = mPath;
        if (isAssetPath(mPath)) {
            data.mAssetData = AAssetDir_getNextFileName((AAssetDir *)mHandle);
            return data.mAssetData != nullptr;
        } else {
            data.mData = readdir((DIR *)mHandle);

            if (!data.mData)
                return false;
            return skipSymbolic(mHandle, data);
        }
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
        if (isAssetPath(data.mPath))
            return data.mAssetData;
        else
            return data.mData->d_name;
    }

}
}

#endif