#pragma once

namespace Engine {
namespace Render {
    namespace OpenGLShaderCodeGen {

        void generate(std::ostream &stream, const CodeGen::ShaderFile &file, ShaderType index);

    }
}
}