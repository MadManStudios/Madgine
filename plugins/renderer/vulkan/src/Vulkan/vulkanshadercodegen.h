#pragma once

namespace Engine {
namespace Render {
    namespace VulkanShaderCodeGen {
    
        void generate(std::ostream &stream, const CodeGen::ShaderFile &file, uint32_t index);
    
    }
}
}