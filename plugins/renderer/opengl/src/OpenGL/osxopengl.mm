#include "opengllib.h"

#include "osxopengl.h"

#include <Cocoa/Cocoa.h>

#include "Platform/window/windowapi.h"

namespace Engine{
namespace Render{
namespace OSXBridge{

ContextHandle createContext(Platform::Window::OSWindow *window){
    #define PixelFormatAttrib(...) __VA_ARGS__
      NSOpenGLPixelFormatAttribute attributes[] = {
        PixelFormatAttrib(NSOpenGLPFADoubleBuffer),
        PixelFormatAttrib(NSOpenGLPFAAccelerated),
        PixelFormatAttrib(NSOpenGLPFABackingStore, YES),
        PixelFormatAttrib(NSOpenGLPFAColorSize, 24),
        PixelFormatAttrib(NSOpenGLPFADepthSize, 24),
        PixelFormatAttrib(NSOpenGLPFAAlphaSize, 8),
        PixelFormatAttrib(NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core),
        0
      };
    #undef PixelFormatAttrib
    
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    NSOpenGLView *view = [[NSOpenGLView alloc] initWithFrame:NSRect{} pixelFormat:pixelFormat];
    [static_cast<NSWindow*>(window->ptrHandle()) setContentView:view];
    [[view openGLContext] setView:view];
    return [view openGLContext];
}

void makeCurrent(ContextHandle context){
    [static_cast<NSOpenGLContext*>(context) makeCurrentContext];
}

void resetContext(){
    [NSOpenGLContext clearCurrentContext];
}

void destroyContext(ContextHandle context) {
    [static_cast<NSOpenGLContext*>(context) release];
}

void swapBuffers(ContextHandle context) {
    [static_cast<NSOpenGLContext*>(context) flushBuffer];
}

}
}
}
