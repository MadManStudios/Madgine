#pragma once

namespace Engine {

struct InterfacesVector {
    int x;
    int y;

    InterfacesVector operator-(const InterfacesVector& other) const {
        return { x - other.x, y - other.y };
    }
    InterfacesVector operator+(const InterfacesVector &other) const
    {
        return { x + other.x, y + other.y };
    }
};

namespace Debug {

    struct TraceBack;
    struct StackTraceIterator;

    namespace Memory {
        struct StatsMemoryResource;
        struct MemoryTracker;
    }

}

namespace Window {
    struct WindowEventListener;
    struct OSWindow;
    struct WindowSettings;
}

namespace Filesystem {
    struct Path;
    struct FileQuery;
    struct FileQueryState;
    struct FileQueryResult;
}

namespace Dl {
    struct SharedLibraryQuery;
    struct SharedLibraryQueryState;
}

namespace Input {
    struct KeyEventArgs;
    struct PointerEventArgs;
    struct AxisEventArgs;

    namespace MouseButton {
        enum MouseButton : unsigned char;
    }

    namespace Key {
        enum Key : uint8_t;
    }
}

namespace Log {

    struct Log;

    struct LogListener;
    struct StandardLog;
}

}

