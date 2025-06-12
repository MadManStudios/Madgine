#pragma once

#include "shaderobject.h"

#include "Modules/threading/task.h"

namespace Engine {
namespace Render {

    struct MADGINE_RENDER_EXPORT ShaderCache {

        static Filesystem::Path directory();

        static Threading::Task<void> generate(const Filesystem::Path &path, ShaderObjectPtr object, std::string_view target, std::string_view profile);
    };

}
}

REGISTER_TYPE(Engine::Render::ShaderCache)