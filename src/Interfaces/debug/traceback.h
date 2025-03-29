#pragma once

#ifndef NDEBUG

namespace Engine {
namespace Debug {
    struct TraceBack {

        TraceBack(void *addr = nullptr, const char *func = "????????", const char *file = "", int line = -1)
            : mFile(file)
            , mLineNr(line)
            , mFunction(func)
            , mAddress(addr)
        {
        }

        bool operator==(const TraceBack &other) const
        {
            return strcmp(mFile, other.mFile) == 0 && strcmp(mFunction, other.mFunction) == 0 /* && mLineNr == other.mLineNr*/;
        }

        const char *mFile;
        int mLineNr;
        const char *mFunction;
        void *mAddress;
    };

    INTERFACES_EXPORT std::ostream &operator<<(std::ostream &out, const TraceBack &trace);
}
}

namespace std {
INTERFACES_EXPORT string to_string(const Engine::Debug::TraceBack &t);

template <>
struct hash<Engine::Debug::TraceBack> {

    static size_t hashCString(const char *s)
    {
        // http://www.cse.yorku.ca/~oz/hash.html
        const char *e = s + 16;
        size_t h = 5381;
        int c;
        while ((c = *s++) && s != e)
            h = ((h << 5) + h) + c;
        return h;
    }

    size_t operator()(const Engine::Debug::TraceBack &traceback) const
    {        
        return 1647 * hashCString(traceback.mFile) - hashCString(traceback.mFunction);
    }
};
}

#endif