#include "../interfaceslib.h"

#include "windowapi.h"
#include "windowsettings.h"

#include <UIKit/UIKit.h>

#if __has_feature(objc_arc)
#error "ARC is on!"
#endif

namespace Engine{
namespace Window{
struct IOSWindow;
}
}

/*@interface Cocoa_WindowListener : NSObject <UIWindowDelegate>{
    Engine::Window::OSXWindow *mWindow;
}

- (void)listen:(Engine::Window::OSXWindow *)window;
@end*/

namespace Engine {
namespace Window {

    DLL_EXPORT const PlatformCapabilities platformCapabilities {
        true,
        static_cast<float>([[UIScreen mainScreen] scale])
    };


    struct IOSWindow : OSWindow {
        IOSWindow(UIWindow *handle, WindowEventListener *listener)
            : OSWindow((uintptr_t)handle, listener)
            //, mListener([[Cocoa_WindowListener alloc] init])
        {
            //[mListener listen:this];
        }
        
        ~IOSWindow(){
            //[mListener dealloc];
        }

        /*bool handle(const XEvent &e)
        {
            switch (e.type) {
            case ConfigureNotify: {
                const XConfigureEvent &xce = e.xconfigure;

                /* This event type is generated for a variety of
                       happenings, so check whether the window has been
                       resized. */
/*
                if (xce.width != mWidth || xce.height != mHeight) {
                    mWidth = xce.width;
                    mHeight = xce.height;
                    onResize(mWidth, mHeight);
                }
                break;
            }
            case ClientMessage: {
                const XClientMessageEvent &xcme = e.xclient;
                if (xcme.data.l[0] == WM_DELETE_WINDOW) {
                    onClose();
                }
                break;
            }
            case DestroyNotify:
                return false;
            }
            return true;
        }*/

        InterfacesVector mLastKnownMousePos;
        //Cocoa_WindowListener *mListener;
    };

         InterfacesVector OSWindow::size() 
        {
            CGRect rect = [reinterpret_cast<UIWindow*>(mHandle) frame];
            return {static_cast<int>(rect.size.width), static_cast<int>(rect.size.height)};
        }

         InterfacesVector OSWindow::renderSize() 
        {
            //TODO
            return {static_cast<int>(size().x * platformCapabilities.mScalingFactor), static_cast<int>(size().y * platformCapabilities.mScalingFactor)};
        }

         InterfacesVector OSWindow::pos() 
        {
            CGRect rect = [reinterpret_cast<UIWindow*>(mHandle) frame];
            return {static_cast<int>(rect.origin.x), static_cast<int>([[reinterpret_cast<UIWindow*>(mHandle) screen] bounds].size.height - rect.origin.y - rect.size.height)};
        }

         InterfacesVector OSWindow::renderPos() 
        {
            return {static_cast<int>(pos().x * platformCapabilities.mScalingFactor), static_cast<int>(pos().y * platformCapabilities.mScalingFactor)};
        }

         void OSWindow::setSize(const InterfacesVector &size) 
        {
            CGRect rect{static_cast<double>(pos().x), static_cast<double>(pos().y), static_cast<double>(size.x), static_cast<double>(size.y)};
            [reinterpret_cast<UIWindow*>(mHandle) setBounds:rect];
        }

         void OSWindow::setRenderSize(const InterfacesVector &size) 
        {
            setSize({static_cast<int>(size.x / platformCapabilities.mScalingFactor), static_cast<int>(size.y / platformCapabilities.mScalingFactor)});
        }

         void OSWindow::setPos(const InterfacesVector &size) 
        {
        }

         void OSWindow::setRenderPos(const InterfacesVector &size) 
        {
        }

         void OSWindow::show() 
        {
            [reinterpret_cast<UIWindow*>(mHandle) makeKeyAndVisible];
        }

         bool OSWindow::isMinimized() 
        {
            return false;
        }

         void OSWindow::focus() 
        {
        }

         bool OSWindow::hasFocus() 
        {
            return true;
        }

         void OSWindow::setTitle(const char *title) 
        {
        }

        void OSWindow::close()
        {

        }

         void OSWindow::destroy() 
        {
            [reinterpret_cast<UIWindow*>(mHandle) close];
        }
        
         void OSWindow::captureInput() 
        {
            
        }
        
         void OSWindow::releaseInput() 
        {
            
        }

         bool OSWindow::isMaximized() 
        {
            return false;
        }
        
         bool OSWindow::isFullscreen() 
        {
            return false;
        }
        
         void OSWindow::update() 
        {
            const CFTimeInterval seconds = 0.000002;

            SInt32 result;
            do {
                result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, TRUE);
            } while (result == kCFRunLoopRunHandledSource);

            do {
                result = CFRunLoopRunInMode((CFStringRef)UITrackingRunLoopMode, seconds, TRUE);
            } while(result == kCFRunLoopRunHandledSource);
        }
        
         bool OSWindow::isKeyDown(Input::Key::Key key) 
        {
            return false;
        }

         std::string OSWindow::title() const  {
            return "";
        }
        
        void OSWindow::setCursorIcon(Input::CursorIcon icon)
        {
            /*SetCursor(LoadCursor(NULL, [](Input::CursorIcon icon) {
                switch (icon) {
                case Input::CursorIcon::Arrow:
                    return IDC_ARROW;
                case Input::CursorIcon::TextInput:
                    return IDC_IBEAM;
                case Input::CursorIcon::ResizeAll:
                    return IDC_SIZEALL;
                case Input::CursorIcon::ResizeNS:
                    return IDC_SIZENS;
                case Input::CursorIcon::ResizeEW:
                    return IDC_SIZEWE;
                case Input::CursorIcon::ResizeNESW:
                    return IDC_SIZENESW;
                case Input::CursorIcon::ResizeNWSE:
                    return IDC_SIZENWSE;
                case Input::CursorIcon::Hand:
                    return IDC_HAND;
                case Input::CursorIcon::NotAllowed:
                    return IDC_NO;
                default:
                    throw 0;
                }
            }(icon)));*/
        }
        
        std::string OSWindow::getClipboardString()
        {
            return "";
        }

        bool OSWindow::setClipboardString(std::string_view s)
        {
            return true;
        }

        WindowData OSWindow::data() {
            return {};
        }


    static std::unordered_map<UIWindow *, IOSWindow> sWindows;

    OSWindow *sCreateWindow(const WindowSettings &settings, WindowEventListener *listener)
    {
        CGRect rect = CGRectMake(0, 0, settings.mData.mSize.x, settings.mData.mSize.y);
        if (settings.mData.mPosition.x != -1 || settings.mData.mPosition.y != -1) {
            rect.origin.x = settings.mData.mPosition.x;
            rect.origin.y = settings.mData.mPosition.y;
        }
        
        UIWindow *handle = [[UIWindow alloc] initWithFrame:rect];


        //[handle setTitle:[NSString stringWithUTF8String:settings.mTitle]];
        
        //[handle cascadeTopLeftFromPoint:NSMakePoint(20,20)];
        [handle makeKeyAndVisible];
        
        auto pib = sWindows.try_emplace(handle, handle, listener);
        assert(pib.second);
        
        return &pib.first->second;
    }

    static std::vector<MonitorInfo> sBuffer;

    static void updateMonitors()
    {
        sBuffer.clear();
        for (UIScreen *screen in [UIScreen screens]){
            CGRect frame = [screen bounds];
            sBuffer.push_back({static_cast<int>(frame.origin.x), static_cast<int>(frame.origin.y), static_cast<int>(frame.size.width), static_cast<int>(frame.size.height)});
        }
    }
    
    std::vector<MonitorInfo> listMonitors()
    {
        if (sBuffer.empty())
            updateMonitors();
        return sBuffer;
    }


}
}
