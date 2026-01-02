#pragma once

#include "Modules/threading/task.h"

#include "shaderobject.h"

namespace Engine {
namespace Render {

    struct MADGINE_RENDER_EXPORT ShaderCache {

        static Filesystem::Path directory();

        static Threading::Task<bool> generate(const Filesystem::Path &path, ShaderObjectPtr object, std::string_view target, ShaderType type);

        static void registerShader(ShaderObjectPtr (*object)());
        static std::list<ShaderObjectPtr> shaderCache();
    };

}
}

#define CACHED_SHADER(Object) static Engine::Guard sGuard##__LINE__ { []() { Engine::Render::ShaderCache::registerShader([]() -> Engine::Render::ShaderObjectPtr { return Object; }); } };