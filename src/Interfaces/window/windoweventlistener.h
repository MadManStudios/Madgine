#pragma once

namespace Engine {
namespace Window {

    struct WindowEventListener {
        virtual bool onWindowEvent(const WindowEvent &event) = 0;
    };

}
}