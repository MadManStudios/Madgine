#include "../interfaceslib.h"

#include "windowapi.h"
#include "windowsettings.h"

#include <Cocoa/Cocoa.h>

#if __has_feature(objc_arc)
#error "ARC is on!"
#endif

namespace Engine{
namespace Window{
struct OSXWindow;
}
}

@interface Cocoa_WindowListener : NSObject <NSWindowDelegate>{
    Engine::Window::OSXWindow *mWindow;
}

- (void)listen:(Engine::Window::OSXWindow *)window;
@end

namespace Engine {
namespace Window {

    DLL_EXPORT const PlatformCapabilities platformCapabilities {
        true,
        static_cast<float>([[NSScreen mainScreen] backingScaleFactor])
    };


    struct OSXWindow : OSWindow {
        OSXWindow(NSWindow *handle, WindowEventListener *listener)
            : OSWindow((uintptr_t)handle, listener)
            , mListener([[Cocoa_WindowListener alloc] init])
        {
            [mListener listen:this];
        }
        
        ~OSXWindow(){
            [mListener dealloc];
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
        
        using OSWindow::onResize;
        
        InterfacesVector mLastKnownMousePos;
        Cocoa_WindowListener *mListener;
    };

        InterfacesVector OSWindow::size()
    {
        NSRect rect = [reinterpret_cast<NSWindow*>(mHandle) contentRectForFrameRect:[reinterpret_cast<NSWindow*>(mHandle) frame]];
        return {static_cast<int>(rect.size.width), static_cast<int>(rect.size.height)};
    }

        InterfacesVector OSWindow::renderSize()
    {
        //TODO
        return {static_cast<int>(size().x * platformCapabilities.mScalingFactor), static_cast<int>(size().y * platformCapabilities.mScalingFactor)};
    }

        InterfacesVector OSWindow::pos()
    {
        NSRect rect = [reinterpret_cast<NSWindow*>(mHandle) contentRectForFrameRect:[reinterpret_cast<NSWindow*>(mHandle) frame]];
        return {static_cast<int>(rect.origin.x), static_cast<int>([[reinterpret_cast<NSWindow*>(mHandle) screen] frame].size.height - rect.origin.y - rect.size.height)};
    }

        InterfacesVector OSWindow::renderPos()
    {
        return {static_cast<int>(pos().x * platformCapabilities.mScalingFactor), static_cast<int>(pos().y * platformCapabilities.mScalingFactor)};
    }

        void OSWindow::setSize(const InterfacesVector &size)
    {
        NSRect rect{static_cast<double>(pos().x), static_cast<double>(pos().y), static_cast<double>(size.x), static_cast<double>(size.y)};
        [reinterpret_cast<NSWindow*>(mHandle) setFrame: rect display: YES animate: NO];
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
        [reinterpret_cast<NSWindow*>(mHandle) makeKeyAndOrderFront:NSApp];
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
        [reinterpret_cast<NSWindow*>(mHandle) close];
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

        void OSWindow::update()
    {
        NSEvent* ev;
        while (NULL != (ev = [NSApp nextEventMatchingMask: NSAnyEventMask untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES]))
        {
            switch([ev type]){
                case NSEventTypeMouseMoved:
                case NSEventTypeLeftMouseUp:
                case NSEventTypeLeftMouseDown:
                case NSEventTypeLeftMouseDragged:
                {
                    const NSPoint screenLocation = [NSEvent mouseLocation];
                    InterfacesVector screenPos {static_cast<int>(platformCapabilities.mScalingFactor * screenLocation.x), static_cast<int>(platformCapabilities.mScalingFactor * ([[reinterpret_cast<NSWindow*>(mHandle) screen] frame].size.height - screenLocation.y))};
                    const NSPoint windowLocation = [reinterpret_cast<NSWindow*>(mHandle) convertPointFromScreen:screenLocation];
                    InterfacesVector windowPos {static_cast<int>(platformCapabilities.mScalingFactor * windowLocation.x), renderSize().y - static_cast<int>(platformCapabilities.mScalingFactor * windowLocation.y)};
                    
                    switch([ev type]){
                        case NSEventTypeMouseMoved:
                        case NSEventTypeLeftMouseDragged:
                            injectPointerMove(Input::PointerEventArgs{
                                windowPos,
                                screenPos,
                                windowPos - static_cast<OSXWindow*>(this)->mLastKnownMousePos });
                            static_cast<OSXWindow*>(this)->mLastKnownMousePos = windowPos;
                            break;
                        case NSEventTypeLeftMouseDown:
                            injectPointerPress(Input::PointerEventArgs{
                                windowPos, screenPos, Input::MouseButton::LEFT_BUTTON
                            });
                            break;
                        case NSEventTypeLeftMouseUp:
                            injectPointerRelease(Input::PointerEventArgs{
                                windowPos, screenPos, Input::MouseButton::LEFT_BUTTON
                            });
                            break;
                        default:
                            std::terminate();
                    }
                }
                    break;
                default:
                    LOG_WARNING("Unhandled event: " << [ev type]);
            }
            
            [NSApp sendEvent: ev];
        }
    }
    
    bool OSWindow::isKeyDown(Input::Key::Key key)
    {
        return false;
    }
    
    std::string OSWindow::title() const
    {
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

    WindowData OSWindow::data()
    {
        return {};
    }

}
}


@implementation Cocoa_WindowListener
- (void)listen:(Engine::Window::OSXWindow *)window
{
    mWindow = window;
    [reinterpret_cast<NSWindow*>(window->mHandle) setDelegate:self];
}

- (void)windowDidResize:(NSNotification *)aNotification
{
    mWindow->onResize(mWindow->size());
}
@end

namespace Engine{
namespace Window{

    static std::unordered_map<NSWindow *, OSXWindow> sWindows;

    OSWindow *sCreateWindow(const WindowSettings &settings, WindowEventListener *listener)
    {
        NSRect rect = NSMakeRect(0, 0, settings.mData.mSize.x, settings.mData.mSize.y);
        if (settings.mData.mPosition.x != -1 || settings.mData.mPosition.y != -1) {
            rect.origin.x = settings.mData.mPosition.x;
            rect.origin.y = settings.mData.mPosition.y;
        }
        
        id menubar = [NSMenu new];
            id appMenuItem = [NSMenuItem new];
            [menubar addItem:appMenuItem];
            [NSApp setMainMenu:menubar];
            id appMenu = [NSMenu new];
            id appName = [[NSProcessInfo processInfo] processName];
            id quitTitle = [@"Quit " stringByAppendingString:appName];
            id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle
            action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
            [appMenu addItem:quitMenuItem];
            [appMenuItem setSubmenu:appMenu];
        
        NSWindowStyleMask style = 0;
        
        if (settings.mHeadless){
            style |= NSWindowStyleMaskBorderless;
        }else{
            style |= NSWindowStyleMaskTitled |
                     NSWindowStyleMaskClosable |
                     NSWindowStyleMaskResizable |
                     NSWindowStyleMaskMiniaturizable;
        }
        
        NSWindow *handle = [[NSWindow alloc] initWithContentRect:rect
                                             styleMask:style
                                             backing:NSBackingStoreBuffered
                                             defer:NO];


        [handle setTitle:[NSString stringWithUTF8String:settings.mTitle]];
        
        [handle cascadeTopLeftFromPoint:NSMakePoint(20,20)];
        [handle makeKeyAndOrderFront:nil];
        
        
        auto pib = sWindows.try_emplace(handle, handle, listener);
        assert(pib.second);
        
        return &pib.first->second;
    }

    static std::vector<MonitorInfo> sBuffer;

    static void updateMonitors()
    {
        sBuffer.clear();
        for (NSScreen *screen in [NSScreen screens]){
            NSRect frame = [screen frame];
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
