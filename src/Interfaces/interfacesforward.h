#pragma once

namespace Engine {

struct InterfacesVector {
    int x;
    int y;

    InterfacesVector operator-(const InterfacesVector &other) const
    {
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
    struct KeyPressEvent;
    struct KeyReleaseEvent;
    struct PointerPressEvent;
    struct PointerReleaseEvent;
    struct PointerMoveEvent;
    struct AxisEvent;

    namespace MouseButton {
        enum MouseButton : unsigned char;
    }

    namespace Key {
        enum Key : uint8_t;
    }
}

namespace Window {
    struct OSWindow;
    struct WindowSettings;

    struct ResizeEvent;
    struct CloseEvent;
    struct RepaintEvent;

    using WindowEvent = std::variant<ResizeEvent, CloseEvent, RepaintEvent, Input::KeyPressEvent, Input::KeyReleaseEvent, Input::PointerPressEvent, Input::PointerReleaseEvent, Input::PointerMoveEvent, Input::AxisEvent>;

}

namespace Log {

    struct Log;

    struct LogListener;
    struct StandardLog;
}

}
