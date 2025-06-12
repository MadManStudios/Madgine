#include "opengllib.h"

#include "openglshaderloader.h"

#include "util/openglshader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Interfaces/filesystem/fsapi.h"

#include "openglrendercontext.h"

UNIQUECOMPONENT(Engine::Render::OpenGLShaderLoader)

METATABLE_BEGIN(Engine::Render::OpenGLShaderLoader)
MEMBER(mResources)
METATABLE_END(Engine::Render::OpenGLShaderLoader)

METATABLE_BEGIN_BASE(Engine::Render::OpenGLShaderLoader::Resource, Engine::Resources::ResourceBase)
METATABLE_END(Engine::Render::OpenGLShaderLoader::Resource)

SERIALIZETABLE_BEGIN(Engine::Render::OpenGLShaderLoader::Handle)
SERIALIZETABLE_END(Engine::Render::OpenGLShaderLoader::Handle)

namespace Engine {
namespace Render {

    OpenGLShaderLoader::OpenGLShaderLoader()
        : ResourceLoader({
#if !OPENGL_ES
            ".glsl"
#else
            ".glsl_es"
#endif
        })
    {
    }

    Threading::Task<bool> OpenGLShaderLoader::loadImpl(OpenGLShader &shader, ResourceDataInfo &info)
    {
        std::string_view filename = info.resource()->name();

        ShaderType type;
        if (filename.ends_with("_VS"))
            type = ShaderType::VertexShader;
        else if (filename.ends_with("_PS"))
            type = ShaderType::PixelShader;
        else
            throw 0;

        return generate(shader, info, type);
    }

    Threading::Task<bool> OpenGLShaderLoader::generate(OpenGLShader &shader, ResourceDataInfo &info, ShaderType type, ShaderObjectPtr object)
    {
        const Filesystem::Path &p = info.resource()->path();

        std::string entrypoint = "main";
        if (object) {
            entrypoint = object->entrypoint();
            co_await ShaderCache::generate(p, object, "-GLSL", type == VertexShader ? "vs_6_2" : "ps_6_2");
        }

        if (!Filesystem::exists(p))
            co_return false;

        std::string source = info.resource()->readAsText();

        co_return loadFromSource(shader, info.resource()->path().stem(), source, type, info.resource()->path());
    }

    void OpenGLShaderLoader::unloadImpl(OpenGLShader &shader)
    {
        shader.reset();
    }

    bool OpenGLShaderLoader::loadFromSource(OpenGLShader &shader, std::string_view name, std::string source, ShaderType type, const Filesystem::Path &path)
    {
        OpenGLShader tempShader { type };

        GLuint handle = tempShader.mHandle;

        const char *cSource = source.data();

        glShaderSource(handle, 1, &cSource, NULL);
        glCompileShader(handle);
        // check for shader compile errors
        GLint success;
        char infoLog[512];
        glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(handle, 512, NULL, infoLog);
            Engine::Log::LogDummy { Engine::Log::MessageType::FATAL_TYPE, path.c_str() }
                << "Compilation of " << (type == VertexShader ? "VS" : "PS")
                << " Shader '" << name << "' failed:\n"
                << infoLog;
            return false;
        }

        shader = std::move(tempShader);

        return true;
    }

    Threading::TaskQueue *OpenGLShaderLoader::loadingTaskQueue() const
    {
        return OpenGLRenderContext::renderQueue();
    }

    Threading::TaskFuture<bool> OpenGLShaderLoader::Handle::load(ShaderObjectPtr object, ShaderType type, OpenGLShaderLoader *loader)
    {
        return Base::Handle::create(object->name(), ShaderCache::directory() / (std::string { object->metadata().mPath.stem() } + "_" + (type == VertexShader ? "vs" : "ps") + ".glsl"), [object, type](OpenGLShaderLoader *loader, OpenGLShader &shader, ResourceDataInfo &info) { return loader->generate(shader, info, type, object); }, loader);
    }
}
}
