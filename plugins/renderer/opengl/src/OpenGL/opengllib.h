#pragma once

/// @cond

#include "Madgine/renderlib.h"
#include "Madgine/meshloaderlib.h"

#if defined(OpenGL_EXPORTS)
#    define MADGINE_OPENGL_EXPORT DLL_EXPORT
#else
#    define MADGINE_OPENGL_EXPORT DLL_IMPORT
#endif

#include "openglforward.h"

#if !ANDROID && !EMSCRIPTEN && !IOS
#    include "../glad/glad.h"
//#    define OPENGL_ES 30
#elif IOS
#    include <OpenGLES/ES3/gl.h>
#    define OPENGL_ES 30
#elif EMSCRIPTEN
#    include <GLES3/gl3.h>
#    define OPENGL_ES 30
#else
#    include <GLES3/gl31.h>
#    define OPENGL_ES 31
#endif

#if EMSCRIPTEN
#    define WEBGL 1
#else
#    define WEBGL 0
#endif

MADGINE_OPENGL_EXPORT void glDump();

inline void glCheck()
{
    int e = glGetError();
    if (e) {
        {
            std::stringstream out;
            out << "GL-Error: ";
            switch (e) {
                CONSTANT_CASE(GL_INVALID_ENUM, out)
                CONSTANT_CASE(GL_INVALID_VALUE, out)
                CONSTANT_CASE(GL_INVALID_FRAMEBUFFER_OPERATION, out)
                CONSTANT_CASE(GL_INVALID_OPERATION, out)
            default:
                out << "UNKNOWN";
            }
            out << "(" << e << ")";
            LOG_FATAL(out.str());
        }
        glDump();
        std::terminate();
    }
}

#if !EMSCRIPTEN
#    define GL_CHECK() glCheck()
#else
#    define GL_CHECK()
#endif
#define GL_LOG(x) LOG_DEBUG("GL: " << x)

#if OPENGL_ES
#    define CUBE_FRAMEBUFFER (OPENGL_ES >= 32)
#else
#    define CUBE_FRAMEBUFFER 1
#endif

#if OPENGL_ES
#    define MULTISAMPLING (OPENGL_ES >= 31)
#else
#    define MULTISAMPLING 1
#endif

#if WINDOWS

#    define NOMINMAX
#    include <Windows.h>

#    undef ERROR
typedef HGLRC ContextHandle;
typedef HDC SurfaceHandle;

#elif LINUX

struct __GLXcontextRec;
typedef struct __GLXcontextRec *GLXContext;

typedef GLXContext ContextHandle;
typedef uintptr_t SurfaceHandle;

#elif ANDROID || EMSCRIPTEN

typedef void *EGLContext;
typedef void *EGLSurface;
typedef EGLContext ContextHandle;
typedef EGLSurface SurfaceHandle;

#elif OSX

typedef void *ContextHandle;
typedef Engine::Window::OSWindow *SurfaceHandle;

#elif IOS

typedef void *ContextHandle;
typedef Engine::Window::OSWindow *SurfaceHandle;

#else

#    error "Unsupported Platform!"

#endif

/// @endcond
