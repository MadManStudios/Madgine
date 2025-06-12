#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Modules/threading/workgroupstorage.h"

#include "util/openglshader.h"

#include "Madgine/render/shadercache.h"

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

            Threading::TaskFuture<bool> load(ShaderObjectPtr object, ShaderType type, OpenGLShaderLoader *loader = &OpenGLShaderLoader::getSingleton());
        };


        Threading::Task<bool> loadImpl(OpenGLShader &shader, ResourceDataInfo &info);
        Threading::Task<bool> generate(OpenGLShader &shader, ResourceDataInfo &info, ShaderType type, ShaderObjectPtr object = {});
        void unloadImpl(OpenGLShader &shader);

        bool loadFromSource(OpenGLShader &shader, std::string_view name, std::string source, ShaderType type, const Filesystem::Path &path = {});

        virtual Threading::TaskQueue *loadingTaskQueue() const override;
    };

}
}

REGISTER_TYPE(Engine::Render::OpenGLShaderLoader)