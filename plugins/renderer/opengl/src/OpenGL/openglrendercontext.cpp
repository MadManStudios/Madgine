#include "opengllib.h"

#include "openglrenderwindow.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "openglrendercontext.h"
#include "openglrendertexture.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Interfaces/window/windowapi.h"

#include "openglshaderloader.h"

#include "openglmeshloader.h"
#include "openglpipelineloader.h"
#include "opengltextureloader.h"

#if WINDOWS
#    include "wglext.h"

static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;

#elif LINUX
#    include <GL/glx.h>
#    include <X11/Xlib.h>
namespace Engine {
namespace Window {
    extern Display *sDisplay();
}
}
#elif ANDROID || EMSCRIPTEN
#    include <EGL/egl.h>
#    include <EGL/eglext.h>

EGLDisplay sDisplay = EGL_NO_DISPLAY;

static struct DisplayGuard {
    DisplayGuard()
    {
        sDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (sDisplay != EGL_NO_DISPLAY) {
            if (!eglInitialize(sDisplay, nullptr, nullptr))
                sDisplay = EGL_NO_DISPLAY;
        }
    }

    ~DisplayGuard()
    {
        if (sDisplay != EGL_NO_DISPLAY)
            eglTerminate(sDisplay);
    }
} sDisplayGuard;

#    define X(VAL)    \
        {             \
            VAL, #VAL \
        }
static constexpr struct {
    EGLint attribute;
    const char *name;
} eglAttributeNames[] = {
    X(EGL_BUFFER_SIZE),
    X(EGL_RED_SIZE),
    X(EGL_GREEN_SIZE),
    X(EGL_BLUE_SIZE),
    X(EGL_ALPHA_SIZE),
    X(EGL_CONFIG_CAVEAT),
    X(EGL_CONFIG_ID),
    X(EGL_DEPTH_SIZE),
    X(EGL_LEVEL),
    X(EGL_MAX_PBUFFER_WIDTH),
    X(EGL_MAX_PBUFFER_HEIGHT),
    X(EGL_MAX_PBUFFER_PIXELS),
    X(EGL_NATIVE_RENDERABLE),
    X(EGL_NATIVE_VISUAL_ID),
    X(EGL_NATIVE_VISUAL_TYPE),
    // X(EGL_PRESERVED_RESOURCES),
    X(EGL_SAMPLE_BUFFERS),
    X(EGL_SAMPLES),
    // X(EGL_STENCIL_BITS),
    X(EGL_SURFACE_TYPE),
    X(EGL_TRANSPARENT_TYPE),
    // X(EGL_TRANSPARENT_RED),
    // X(EGL_TRANSPARENT_GREEN),
    // X(EGL_TRANSPARENT_BLUE)
};
#    undef X
#elif OSX
#    include "osxopengl.h"
#elif IOS
#    include "iosopengl.h"
#endif

UNIQUECOMPONENT(Engine::Render::OpenGLRenderContext)

METATABLE_BEGIN(Engine::Render::OpenGLRenderContext)
METATABLE_END(Engine::Render::OpenGLRenderContext)

namespace Engine {
namespace Render {

#if !ANDROID && !EMSCRIPTEN && !OSX && !IOS
    static void __stdcall glDebugOutput(GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar *message,
        const void *userParam)
    {
        // ignore non-significant error/warning codes
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
            return;

        Log::MessageType lvl;
        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            lvl = Log::MessageType::ERROR_TYPE;
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            lvl = Log::MessageType::WARNING_TYPE;
            break;
        case GL_DEBUG_SEVERITY_LOW:
            lvl = Log::MessageType::INFO_TYPE;
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            lvl = Log::MessageType::DEBUG_TYPE;
            break;
        }

        Log::LogDummy cout(lvl);
        cout << "Debug message (" << id << "): " << message << "\n";

        switch (source) {
        case GL_DEBUG_SOURCE_API:
            cout << "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            cout << "Source: Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            cout << "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            cout << "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            cout << "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            cout << "Source: Other";
            break;
        }
        cout << "\n";

        switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            cout << "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            cout << "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            cout << "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            cout << "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            cout << "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            cout << "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            cout << "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            cout << "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            cout << "Type: Other";
            break;
        }
        cout << "\n";
    }
#endif

    void resetContext()
    {
        GL_LOG("Resetting Context");
#if WINDOWS
        wglMakeCurrent(NULL, NULL);
#elif LINUX
        glXMakeCurrent(Window::sDisplay(), None, NULL);
#elif ANDROID || EMSCRIPTEN
        eglMakeCurrent(sDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
#elif OSX
        OSXBridge::resetContext();
#elif IOS
        IOSBridge::resetContext();
#endif
    }

    void makeCurrent(SurfaceHandle surface, ContextHandle context)
    {
        GL_LOG("Setting Context: " << context);
#if WINDOWS
        if (!wglMakeCurrent(surface, context)) {
            LOG_ERROR("Error-Code: " << GetLastError());
            std::terminate();
        }
#elif LINUX
        if (!glXMakeCurrent(Window::sDisplay(), surface, context)) {
            LOG_ERROR("Error-Code: " << errno);
            std::terminate();
        }
#elif ANDROID || EMSCRIPTEN
        if (!eglMakeCurrent(sDisplay, surface, surface, context)) {
            LOG_ERROR("Error-Code: " << eglGetError());
            std::terminate();
        }
#elif OSX
        OSXBridge::makeCurrent(context);
#elif IOS
        IOSBridge::makeCurrent(context);
#endif
    }

    void swapBuffers(SurfaceHandle surface, ContextHandle context)
    {
#if WINDOWS
        SwapBuffers(surface);
#elif LINUX
        glXSwapBuffers(Window::sDisplay(), surface);
#elif ANDROID || EMSCRIPTEN
        eglSwapBuffers(sDisplay, context);
#elif OSX
        OSXBridge::swapBuffers(context);
#elif IOS
        IOSBridge::swapBuffers(context);
#else
#    error "Unsupported Platform!"
#endif
    }

    static bool checkMultisampling()
    {
#if MULTISAMPLING
#    if ANDROID || EMSCRIPTEN
        const EGLint attribs[] = {
            EGL_SAMPLE_BUFFERS, 1,
            EGL_NONE
        };

        EGLint numConfigs;

        return eglChooseConfig(sDisplay, attribs, nullptr, 0, &numConfigs);
#    else
        return glTexImage2DMultisample;
#    endif
#else
        return false;
#endif
    }

    ContextHandle createContext(SurfaceHandle surface, size_t samples, ContextHandle reusedContext, bool setup)
    {

#if WINDOWS
        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR),
            1,
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Flags
            PFD_TYPE_RGBA, // The kind of framebuffer. RGBA or palette.
            32, // Colordepth of the framebuffer.
            0, 0, 0, 0, 0, 0,
            0,
            0,
            0,
            0, 0, 0, 0,
            24, // Number of bits for the depthbuffer
            8, // Number of bits for the stencilbuffer
            0, // Number of Aux buffers in the framebuffer.
            PFD_MAIN_PLANE,
            0,
            0, 0, 0
        };

        int format;
        if (wglChoosePixelFormatARB) {
            float pfAttribFList[] = { 0, 0 };
            int piAttribIList[] = {
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_COLOR_BITS_ARB, 32,
                WGL_RED_BITS_ARB, 8,
                WGL_GREEN_BITS_ARB, 8,
                WGL_BLUE_BITS_ARB, 8,
                WGL_ALPHA_BITS_ARB, 8,
                WGL_DEPTH_BITS_ARB, 24,
                WGL_STENCIL_BITS_ARB, 0,
                WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
                WGL_SAMPLES_ARB, static_cast<int>(samples),
                0, 0
            };

            UINT numFormats;
            bool result = wglChoosePixelFormatARB(surface, piAttribIList, pfAttribFList, 1, &format, &numFormats);
            assert(result);
            assert(numFormats > 0);
        } else {
            format = ChoosePixelFormat(surface, &pfd);
        }
        SetPixelFormat(surface, format, &pfd);

        HGLRC context;
        if (reusedContext) {
            context = reusedContext;
        } else {
            if (wglCreateContextAttribsARB) {

                int attribs[] = {
                    WGL_CONTEXT_PROFILE_MASK_ARB, GL_TRUE,
                    0
                };

                context = wglCreateContextAttribsARB(surface, NULL, attribs);
            } else {
                context = wglCreateContext(surface);
            }
        }

#elif LINUX
        GLXContext context;
        if (reusedContext) {
            context = reusedContext;
        } else {
            static GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };

            static XVisualInfo *vi = glXChooseVisual(Window::sDisplay(), 0, att);
            assert(vi);

            context = glXCreateContext(Window::sDisplay(), vi, /*sharedContext*/ nullptr, GL_TRUE);
        }

#elif ANDROID || EMSCRIPTEN

        EGLContext context;
        if (reusedContext) {
            context = reusedContext;
        } else {
            const EGLint attribs[] = {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
                EGL_GL_COLORSPACE_KHR, EGL_GL_COLORSPACE_SRGB_KHR,
                /* EGL_SAMPLE_BUFFERS, 1,
                EGL_SAMPLES, static_cast<EGLint>(samples),*/
                EGL_NONE
            };

            const EGLint contextAttribs[] = {
                EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL_NONE
            };

            EGLConfig config;
            EGLint numConfigs;

            if (!eglChooseConfig(sDisplay, attribs, &config, 1, &numConfigs)) {
                LOG_ERROR("Could not find suitable EGL configuration");

                bool result = eglGetConfigs(sDisplay, NULL, 0, &numConfigs);
                assert(result);

                LOG("Number of EGL configuration: " << numConfigs);

                std::unique_ptr<EGLConfig[]> configs = std::make_unique<EGLConfig[]>(numConfigs);

                result = eglGetConfigs(sDisplay, configs.get(), numConfigs, &numConfigs);
                assert(result);

                for (int i = 0; i < numConfigs; i++) {
                    Log::LogDummy out { Log::MessageType::INFO_TYPE };
                    out << "Configuration:\n";
                    EGLConfig config = configs[i];
                    for (int j = 0; j < sizeof(eglAttributeNames) / sizeof(eglAttributeNames[0]); j++) {
                        EGLint value = -1;
                        result = eglGetConfigAttrib(sDisplay, config, eglAttributeNames[j].attribute, &value);
                        if (result) {
                            out << " " << eglAttributeNames[j].name << " = " << value << "\n";
                        }
                    }
                }
                std::terminate();
            }

            context = eglCreateContext(sDisplay, config, /*sharedContext*/ nullptr, contextAttribs);
            if (!context) {
                LOG_ERROR("eglCreateContext - ErrorCode: " << eglGetError());
            }
        }

#elif OSX

        ContextHandle context;
        if (reusedContext) {
            context = reusedContext;
        } else {
            context = OSXBridge::createContext(surface);
        }

#elif IOS

        ContextHandle context;
        if (reusedContext) {
            context = reusedContext;
        } else {
            context = IOSBridge::createContext(window);
        }

#endif

        if (!reusedContext) {
            makeCurrent(surface, context);
        }

        if (!reusedContext && setup) {

#if WINDOWS
            if (wglSwapIntervalEXT)
                wglSwapIntervalEXT(0);
#endif

#if !ANDROID && !EMSCRIPTEN && !OSX && !IOS
            glEnable(GL_DEBUG_OUTPUT);
            GL_CHECK();
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            GL_CHECK();
            glDebugMessageCallback(glDebugOutput, nullptr);
            GL_CHECK();
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            GL_CHECK();
#endif

            // Pos
            glVertexAttrib3f(0, 0, 0, 0);
            GL_CHECK();
            glVertexAttrib1f(1, 1);
            // Pos2
            glVertexAttrib2f(2, 0, 0);
            GL_CHECK();
            // Normal
            glVertexAttrib3f(3, 0, 0, 0);
            GL_CHECK();
            // Color
            glVertexAttrib4f(4, 1, 1, 1, 1);
            GL_CHECK();
            // UV
            glVertexAttrib2f(5, 0, 0);
            GL_CHECK();
            // BoneIndices
            glVertexAttribI4i(6, 0, 0, 0, 0);
            GL_CHECK();
            // Weights
            glVertexAttrib4f(7, 0, 0, 0, 0);
            GL_CHECK();

            glEnable(GL_BLEND);
            GL_CHECK();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            GL_CHECK();

            glEnable(GL_DEPTH_TEST);
            GL_CHECK();
            glDepthMask(GL_TRUE);
            GL_CHECK();
            glDepthFunc(GL_LESS);
            GL_CHECK();
            // glDepthRange(0.0, 1.0);

            glEnable(GL_SCISSOR_TEST);
            GL_CHECK();

            if (glGetString) {
                const GLubyte *val;
#define GL_LOG_PROPERTY(name)         \
    val = glGetString(name);          \
    if (val) {                        \
        LOG_DEBUG(#name ": " << val); \
    }

                GL_LOG_PROPERTY(GL_VERSION);
                GL_LOG_PROPERTY(GL_VENDOR);
                GL_LOG_PROPERTY(GL_RENDERER);
#if !OSX
                GL_LOG_PROPERTY(GL_EXTENSIONS);
#endif
            }
        }

        return context;
    }

    void destroyContext(SurfaceHandle surface, ContextHandle context, bool reusedContext)
    {
        if (!reusedContext)
            resetContext();

#if WINDOWS
        ReleaseDC(WindowFromDC(surface), surface);

        if (!reusedContext)
            wglDeleteContext(context);
#elif LINUX
        if (!reusedContext)
            glXDestroyContext(Window::sDisplay(), context);
#elif ANDROID || EMSCRIPTEN
        if (!reusedContext)
            eglDestroyContext(sDisplay, context);
#elif OSX
        if (!reusedContext)
            OSXBridge::destroyContext(context);
#endif
    }

#if !ANDROID && !EMSCRIPTEN && !IOS
    namespace {
        void OpenGLInit()
        {
            static std::once_flag once;

            std::call_once(once, []() {
                Engine::Window::WindowSettings settings;
                settings.mHidden = true;
                Window::OSWindow *tmp = Window::sCreateWindow(settings, nullptr);
#    if WINDOWS
                SurfaceHandle surface = GetDC((HWND)tmp->mHandle);
#    elif LINUX
                SurfaceHandle surface = tmp->mHandle;
#    elif OSX
                SurfaceHandle surface = tmp;
#    else
#        error "Unsupported Platform!"
#    endif
                ContextHandle context = createContext(surface, 1, nullptr, false);

#    if WINDOWS
                wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
                assert(wglCreateContextAttribsARB);
                wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
                assert(wglChoosePixelFormatARB);
                wglSwapIntervalEXT = reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(wglGetProcAddress("wglSwapIntervalEXT"));
#    endif
                bool result = gladLoadGL();
                assert(result);

                destroyContext(surface, context);
                tmp->destroy();
            });
        }
    }
#endif

    OpenGLRenderContext::OpenGLRenderContext(Threading::TaskQueue *queue)
        : Component(queue)
        , mBufferAllocator(mBufferMemoryHeap)
        , mTempAllocator(mTempMemoryHeap)
#if OPENGL_ES
        , mTempMemoryHeap(GL_UNIFORM_BUFFER)
#    if EMSCRIPTEN
        , mTempIndexMemoryHeap(GL_ELEMENT_ARRAY_BUFFER)
        , mTempIndexAllocator(mTempIndexMemoryHeap)
#    endif
#endif
    {
    }

    OpenGLRenderContext::~OpenGLRenderContext()
    {
    }

    std::unique_ptr<RenderTarget> OpenGLRenderContext::createRenderTexture(const Vector2i &size, const RenderTextureConfig &config)
    {
        return std::make_unique<OpenGLRenderTexture>(this, size, config);
    }

    OpenGLRenderContext &OpenGLRenderContext::getSingleton()
    {
        return static_cast<OpenGLRenderContext &>(RenderContext::getSingleton());
    }

    std::unique_ptr<RenderTarget> OpenGLRenderContext::createRenderWindow(Window::OSWindow *w, size_t samples)
    {
        checkThread();

#if !ANDROID && !EMSCRIPTEN && !IOS
        OpenGLInit();
#endif

        OpenGLRenderWindow *sharedContext = nullptr;
        for (RenderTarget *target : mRenderTargets) {
            sharedContext = dynamic_cast<OpenGLRenderWindow *>(target);
            if (sharedContext)
                break;
        }

        std::unique_ptr<RenderTarget> window = std::make_unique<OpenGLRenderWindow>(this, w, samples, sharedContext);

        return window;
    }

    Threading::Task<void> OpenGLRenderContext::unloadAllResources()
    {
        co_await RenderContext::unloadAllResources();

        for (std::pair<const std::string, OpenGLPipelineLoader::Resource> &res : OpenGLPipelineLoader::getSingleton()) {
            co_await res.second.forceUnload();
        }

        for (std::pair<const std::string, OpenGLTextureLoader::Resource> &res : OpenGLTextureLoader::getSingleton()) {
            co_await res.second.forceUnload();
        }

        for (std::pair<const std::string, OpenGLMeshLoader::Resource> &res : OpenGLMeshLoader::getSingleton()) {
            co_await res.second.forceUnload();
        }
    }

    bool OpenGLRenderContext::supportsMultisampling() const
    {
        return checkMultisampling();
    }

    template <size_t I = 1>
    struct ResourceBlockBuffer {
        size_t mSize;
        const Texture *mTexture[I];
    };

    UniqueResourceBlock OpenGLRenderContext::createResourceBlock(std::vector<const Texture *> textures)
    {
        std::unique_ptr<OpenGLResourceBlock<4>> ptr = std::make_unique<OpenGLResourceBlock<4>>();
        ptr->mSize = textures.size();
        for (size_t i = 0; i < textures.size(); ++i) {
            ptr->mResources[i].mHandle = textures[i]->handle();
            ptr->mResources[i].mTarget = static_cast<const OpenGLTexture *>(textures[i])->target();
        }

        UniqueResourceBlock block;
        block.setupAs<std::unique_ptr<OpenGLResourceBlock<4>>>() = std::move(ptr);
        return block;
    }

    void OpenGLRenderContext::destroyResourceBlock(UniqueResourceBlock &block)
    {
        block.release<std::unique_ptr<OpenGLResourceBlock<4>>>();
    }

    static constexpr GLenum vTypes[] = {
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_FLOAT,
        GL_INT,
        GL_FLOAT
    };

    void OpenGLRenderContext::bindFormat(VertexFormat format, size_t bufferOffset)
    {
#if !OPENGL_ES || OPENGL_ES >= 31

        auto pib = mVAOs.try_emplace(format, create);
        pib.first->second.bind();
        if (!pib.second)
            return;

        GLuint offset = 0;
        for (size_t i = 0; i < VertexElements::size; ++i) {
            if (format.has(i)) {
                if (vTypes[i] == GL_FLOAT)
                    glVertexAttribFormat(i, sVertexElementSizes[i] / 4, vTypes[i], GL_FALSE, offset);
                else
                    glVertexAttribIFormat(i, sVertexElementSizes[i] / 4, vTypes[i], offset);
                GL_CHECK();
                glVertexAttribBinding(i, 0);
                GL_CHECK();
                glEnableVertexAttribArray(i);
                offset += sVertexElementSizes[i];
            } else {
                // glDisableVertexAttribArray(attribIndex);
            }
        }

#else

        GLuint stride = format.stride();

        const std::byte *offset = reinterpret_cast<const std::byte *>(bufferOffset);
        for (size_t i = 0; i < VertexElements::size; ++i) {
            if (format.has(i)) {
                if (vTypes[i] == GL_FLOAT)
                    glVertexAttribPointer(i, sVertexElementSizes[i] / 4, vTypes[i], GL_FALSE, stride, offset);
                else
                    glVertexAttribIPointer(i, sVertexElementSizes[i] / 4, vTypes[i], stride, offset);
                GL_CHECK();
                glEnableVertexAttribArray(i);
                offset += sVertexElementSizes[i];
            } else {
                glDisableVertexAttribArray(i);
            }
        }

        for (size_t i = 0; i < 8; ++i) {
            glDisableVertexAttribArray(VertexElements::size + i);
        }
#endif
    }

    void OpenGLRenderContext::unbindFormat()
    {
#if !OPENGL_ES || OPENGL_ES >= 31
        glBindVertexArray(0);
        GL_CHECK();
#endif
    }

}
}
