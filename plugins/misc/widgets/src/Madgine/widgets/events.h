#pragma once

namespace Engine {
namespace Widgets {

    struct PointerClickEvent {
        mutable InterfacesVector mWindowPosition;
        InterfacesVector mScreenPosition;
        Input::MouseButton::MouseButton mButton;
    };

    struct DragBeginEvent {
        mutable InterfacesVector mWindowPosition;
        InterfacesVector mScreenPosition;
        Input::MouseButton::MouseButton mButton;
    };

    struct DragMoveEvent {
        mutable InterfacesVector mWindowPosition;
        InterfacesVector mScreenPosition;
        InterfacesVector mMoveDelta;
    };

    struct DragEndEvent {
        mutable InterfacesVector mWindowPosition;
        InterfacesVector mScreenPosition;
        Input::MouseButton::MouseButton mButton;
    };

}
}