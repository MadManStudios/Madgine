#pragma once

namespace Engine {
namespace Widgets {

    struct PointerClickEvent {
        mutable Platform::PlatformVector mWindowPosition;
        Platform::PlatformVector mScreenPosition;
        Platform::Input::MouseButton::MouseButton mButton;
    };

    struct DragBeginEvent {
        mutable Platform::PlatformVector mWindowPosition;
        Platform::PlatformVector mScreenPosition;
        Platform::Input::MouseButton::MouseButton mButton;
    };

    struct DragMoveEvent {
        mutable Platform::PlatformVector mWindowPosition;
        Platform::PlatformVector mScreenPosition;
        Platform::PlatformVector mMoveDelta;
    };

    struct DragEndEvent {
        mutable Platform::PlatformVector mWindowPosition;
        Platform::PlatformVector mScreenPosition;
        Platform::Input::MouseButton::MouseButton mButton;
    };

}
}