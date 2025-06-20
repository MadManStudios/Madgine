#pragma once



namespace Engine {

namespace Window {

    struct WindowEventListener {
        virtual void onResize(const InterfacesVector &renderSize) {};
        virtual void onClose() {};
        virtual void onRepaint() {};

        //Input
        virtual bool injectKeyPress(const Input::KeyEventArgs &arg) { return false; };
        virtual bool injectKeyRelease(const Input::KeyEventArgs &arg) { return false; }
        virtual bool injectPointerPress(const Input::PointerEventArgs &arg) { return false; }
        virtual bool injectPointerRelease(const Input::PointerEventArgs &arg) { return false; }
        virtual bool injectPointerMove(const Input::PointerEventArgs &arg) { return false; }
        virtual bool injectAxisEvent(const Input::AxisEventArgs &arg) { return false; }


    };

}

}