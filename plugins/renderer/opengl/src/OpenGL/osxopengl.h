#pragma once

namespace Engine {
namespace Render {
    namespace OSXBridge {

        ContextHandle createContext(Window::OSWindow *window);
        void makeCurrent(ContextHandle context);
        void resetContext();
        void destroyContext(ContextHandle context);
        void swapBuffers(ContextHandle context);

    }
}
}
