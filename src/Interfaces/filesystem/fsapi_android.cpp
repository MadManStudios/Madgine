#include "../interfaceslib.h"

#if ANDROID

#    include "fsapi.h"

#    include <dirent.h>
#    include <sys/stat.h>
#    include <unistd.h>

#    include <android/asset_manager.h>

#include <android/native_activity.h>

namespace Engine {
namespace Filesystem {

    AAssetManager *sAssetManager = nullptr;
    Path sAppData;

    static const char sAssetPrefix[] = "assets:";

    static bool isAssetPath(const Path &p)
    {
        return StringUtil::startsWith(p.str(), sAssetPrefix);
    }

    static const char *assetPath(const Path &p)
    {
        assert(isAssetPath(p));
        const char *dir = p.c_str() + sizeof(sAssetPrefix) - 1;
        if (isSeparator(*dir))
            ++dir;
        return dir;
    }

    void setup(void* _activity) {
        ANativeActivity *activity = static_cast<ANativeActivity *>(_activity);

        sAssetManager = activity->assetManager;
        sAppData = activity->internalDataPath;
    }

    Path executablePath()
    {
        //TODO
        char buffer[512];

        auto result = readlink("/proc/self/exe", buffer, sizeof(buffer));
        assert(result > 0);
        buffer[result] = '\0';

        return Path(buffer).parentPath();
    }

    std::string executableName() {
        //TODO
        char buffer[512];

        auto result = readlink("/proc/self/exe", buffer, sizeof(buffer));
        assert(result > 0);
        buffer[result] = '\0';

        return std::string { Path(buffer).stem() };
    }

    Path appDataPath()
    {
        assert(!sAppData.empty());

        if (!exists(sAppData))
            createDirectory(sAppData);

        return sAppData;
    }

    bool isDir(const Path& p) {
        if (isAssetPath(p))
            return false;

        struct stat statbuffer;
        auto result = stat(p.c_str(), &statbuffer);
        assert(result != -1);
        return (statbuffer.st_mode & S_IFMT) == S_IFDIR;
    }

    bool createDirectory(const Path &p)
    {
        assert(!isAssetPath(p));
        auto result = mkdir(p.c_str(), 0700);
        return result == 0;
    }

    bool exists(const Path &p)
    {
        if (isAssetPath(p)) {
            AAssetDir *dir = AAssetManager_openDir(sAssetManager, assetPath(p));
            bool result = AAssetDir_getNextFileName(dir) != nullptr;
            AAssetDir_close(dir);
            if (result)
                return true;
            AAsset *file = AAssetManager_open(sAssetManager, assetPath(p), AASSET_MODE_RANDOM);
            result = AAsset_getLength(file) > 0;
            AAsset_close(file);
            return result;
        } else {
            return access(p.c_str(), F_OK) != -1;
        }
    }

    bool remove(const Path &p)
    {
        assert(!isAssetPath(p));
        return ::remove(p.c_str());
    }

    void makeNormalized(std::string &p)
    {        
    }

    bool isAbsolute(const Path &p)
    {
        return isAssetPath(p) || p.c_str()[0] == '/';
    }

    bool isEqual(const Path &p1, const Path &p2)
    {
        return p1 == p2;
    }

    struct AAsset_Streambuf : public std::basic_streambuf<char> {
        AAsset_Streambuf(const char *path)
            : mAsset(AAssetManager_open(sAssetManager, path, AASSET_MODE_RANDOM))
        {
            char *begin = (char *)AAsset_getBuffer(mAsset);
            char *end = begin + AAsset_getLength64(mAsset);
            setg(begin, begin, end);
        }

        ~AAsset_Streambuf()
        {
            AAsset_close(mAsset);
        }

        pos_type seekoff(off_type off, std::ios_base::seekdir dir,
            std::ios_base::openmode mode = std::ios_base::in) override
        {
            assert(mode & std::ios_base::in);

            switch (dir) {
            case std::ios_base::beg:
                if (eback() + off > egptr())
                    return pos_type(off_type(-1));
                setg(eback(), eback() + off, egptr());
                break;
            case std::ios_base::cur:
                if (gptr() + off < eback() || gptr() + off > egptr())                     
                    return pos_type(off_type(-1));
                setg(eback(), gptr() + off, egptr());
                break;
            case std::ios_base::end:
                if (egptr() + off < eback())
                    return pos_type(off_type(-1));
                setg(eback(), egptr() + off, egptr());
                break;
            default:
                std::terminate();
            }

            return pos_type(off_type(gptr() - eback()));
        }

        pos_type seekpos(pos_type pos,
            std::ios_base::openmode mode = std::ios_base::in) override
        {
            assert(mode & std::ios_base::in);

            setg(eback(), eback() + pos, egptr());

            return pos_type(off_type(pos));
        }

    private:
        AAsset *mAsset;
    };

    Stream openFileRead(const Path &p, bool isBinary)
    {
        if (isAssetPath(p)) {
            return { std::make_unique<AAsset_Streambuf>(assetPath(p)) };
        } else {
            std::unique_ptr<std::filebuf> buffer = std::make_unique<std::filebuf>();
            if (buffer->open(p.c_str(), static_cast<std::ios_base::openmode>(std::ios_base::in | (isBinary ? std::ios_base::binary : 0))))
                return { std::move(buffer) };
            else
                return {};
        }
    }

    Stream openFileWrite(const Path &p, bool isBinary)
    {
        if (isAssetPath(p)) {
            return { };
        } else {
            std::unique_ptr<std::filebuf> buffer = std::make_unique<std::filebuf>();
            if (buffer->open(p.c_str(), static_cast<std::ios_base::openmode>(std::ios_base::out | (isBinary ? std::ios_base::binary : 0))))
                return { std::move(buffer) };
            else
                return {};
        }
    }

    void setCwd(const Path &p)
    {
        auto result = chdir(p.c_str());
        assert(result == 0);
    }

    Path getCwd()
    {
        char buffer[256];
        auto result = getcwd(buffer, sizeof(buffer));
        assert(result);
        return buffer;
    }

    
    bool isValidPath(const std::string &p)
    {
        const char *c = p.data();
        if (StringUtil::startsWith(p, sAssetPrefix))
            c += strlen(sAssetPrefix);
        for (; c < p.data() + p.size(); ++c)
            if ((!std::isalnum(*c) && !isSeparator(*c) && !std::ispunct(*c) && *c != ' ')
                || *c == '<'
                || *c == '>'
                || *c == ':'
                || *c == '"'
                || *c == '|'
                || *c == '?'
                || *c == '*')
                return false;
        return true;
    }
        
    FileInfo fileInfo(const Path &path)
    {
        struct stat stats;
        stat(path.c_str(), &stats);
        return {
            static_cast<size_t>(stats.st_size)
        };
    }

}
}

#endif