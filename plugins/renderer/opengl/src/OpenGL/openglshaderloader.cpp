#include "opengllib.h"

#include "openglshaderloader.h"

#include "util/openglshader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "openglshadercodegen.h"

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

    Threading::TaskFuture<bool> OpenGLShaderLoader::Ptr::create(const CodeGen::ShaderFile &file, ShaderType type, OpenGLShaderLoader *loader)
    {
        return Base::Ptr::create(
            [=, &file](OpenGLShaderLoader *loader, OpenGLShader &shader) { return loader->create(shader, file, type); }, loader);
    }

    Threading::TaskFuture<bool> OpenGLShaderLoader::Handle::load(std::string_view name, ShaderType type, OpenGLShaderLoader *loader)
    {
        std::string actualName { name };
        switch (type) {
        case ShaderType::PixelShader:
            actualName += "_PS";
            break;
        case ShaderType::VertexShader:
            actualName += "_VS";
            break;
        default:
            throw 0;
        }

        return Base::Handle::load(actualName, loader);
    }

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

    bool OpenGLShaderLoader::loadImpl(OpenGLShader &shader, ResourceDataInfo &info)
    {
        std::string_view filename = info.resource()->name();

        ShaderType type;
        if (filename.ends_with("_VS"))
            type = ShaderType::VertexShader;
        else if (filename.ends_with("_PS"))
            type = ShaderType::PixelShader;
        else
            throw 0;

        std::string source = info.resource()->readAsText();

        return loadFromSource(shader, filename, source, type, info.resource()->path());
    }

    void OpenGLShaderLoader::unloadImpl(OpenGLShader &shader)
    {
        shader.reset();
    }

    bool OpenGLShaderLoader::create(OpenGLShader &shader, const CodeGen::ShaderFile &file, ShaderType type)
    {
        /* if (res->path().empty()) {
            Filesystem::Path dir = Filesystem::appDataPath() / "generated/shader/opengl";

            Filesystem::createDirectories(dir);

            res->setPath(dir / (std::string { res->name() } + (type == VertexShader ? "_VS" : "_PS") + ".glsl"));
        }*/

        std::ostringstream ss;
        OpenGLShaderCodeGen::generate(ss, file, type);

        /* {
            std::ofstream f { res->path() };
            f << ss.str();
        }*/

        return loadFromSource(shader, "<generated>", ss.str(), type, "<generated>");
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

}
}
