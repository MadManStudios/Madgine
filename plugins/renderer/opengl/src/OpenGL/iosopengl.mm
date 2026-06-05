#include "opengllib.h"

#include "iosopengl.h"

#include <UIKit/UIKit.h>
#include <GLKit/GLKit.h>

#include "Interfaces/window/windowapi.h"

namespace Engine{
namespace Render{
namespace IOSBridge{

ContextHandle createContext(Window::OSWindow *window){
    EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    PlatformVector renderSize = window->renderSize();
    CGRect rect = CGRectMake(0, 0, static_cast<double>(renderSize.x), static_cast<double>(renderSize.y));
    GLKView *view = [[GLKView alloc] initWithFrame:rect context:context];
    GLKViewController *viewController = [[GLKViewController alloc] init];
    [viewController setView:view];
    [reinterpret_cast<UIWindow*>(window->mHandle) setRootViewController:viewController];
    
    return context;
}

void makeCurrent(ContextHandle context){
    [EAGLContext setCurrentContext:static_cast<EAGLContext*>(context)];
}

void resetContext(){
    [EAGLContext setCurrentContext:nullptr];
}

void destroyContext(ContextHandle context) {
    [static_cast<EAGLContext*>(context) release];
}

void swapBuffers(ContextHandle context) {
    [reinterpret_cast<EAGLContext*>(context) presentRenderbuffer:GL_RENDERBUFFER];
}

}
}
}
