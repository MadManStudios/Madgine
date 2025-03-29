#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Modules/threading/workgroupstorage.h"

#include "util/openglshader.h"

namespace Engine {
namespace Render {

    struct OpenGLShaderLoader : Resources::ResourceLoader<OpenGLShaderLoader, OpenGLShader, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {
        OpenGLShaderLoader();

        struct Handle : Base::Handle {

            using Base::Handle::Handle;
            Handle(Base::Handle handle)
                : Base::Handle(std::move(handle))
            {
            }

            Threading::TaskFuture<bool> load(std::string_view name, ShaderType type, OpenGLShaderLoader *loader = &OpenGLShaderLoader::getSingleton());
        };

        struct Ptr : Base::Ptr {

            using Base::Ptr::Ptr;
            Ptr(Base::Ptr ptr)
                : Base::Ptr(std::move(ptr))
            {
            }

            Threading::TaskFuture<bool> create(const CodeGen::ShaderFile &file, ShaderType type, OpenGLShaderLoader *loader = &OpenGLShaderLoader::getSingleton());
        };

        bool loadImpl(OpenGLShader &shader, ResourceDataInfo &info);
        void unloadImpl(OpenGLShader &shader);

        bool create(OpenGLShader &shader, const CodeGen::ShaderFile &file, ShaderType type);

        bool loadFromSource(OpenGLShader &shader, std::string_view name, std::string source, ShaderType type, const Filesystem::Path &path = {});

        virtual Threading::TaskQueue *loadingTaskQueue() const override;
    };

}
}

REGISTER_TYPE(Engine::Render::OpenGLShaderLoader)