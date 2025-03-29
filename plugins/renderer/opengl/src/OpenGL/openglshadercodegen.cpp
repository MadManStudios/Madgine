#include "opengllib.h"

#include "openglshadercodegen.h"

#include "Madgine/codegen/codegen_shader.h"

#include "Meta/math/vector4.h"

namespace Engine {
namespace Render {
    namespace OpenGLShaderCodeGen {

        static std::map<ValueTypeDesc, std::string_view> sTypeMap {
            { toValueTypeDesc<Vector4>(), "vec4" },
            { toValueTypeDesc<Vector3>(), "vec3" },
            { toValueTypeDesc<Vector2>(), "vec2" },
            { toValueTypeDesc<Vector4i>(), "ivec4" },
            { toValueTypeDesc<Vector3i>(), "ivec3" },
            { toValueTypeDesc<Vector2i>(), "ivec2" },
            { toValueTypeDesc<Matrix4>(), "mat4" }
        };

        /*std::string generateIncludeGuard(const Engine::BitArray<64> &conditionals, const std::vector<std::string> &conditionalTokenList)
        {
            if (conditionals == Engine::BitArray<64> {})
                return "";

            std::string result = "#if ";

            bool first = true;

            for (size_t i = 0; i < 64; ++i) {
                if (conditionals[i]) {
                    if (first)
                        first = false;
                    else
                        result += " && ";
                    result += "defined(" + conditionalTokenList[i] + ")";
                }
            }

            return result + "\n";
        }*/

        void generateType(std::ostream &stream, const CodeGen::Type &type)
        {
            std::visit(overloaded {
                           [&](const ValueTypeDesc &type) {
                               stream << sTypeMap.at(type);
                           },
                           [&](CodeGen::Struct *structInfo) {
                               stream << structInfo->mName;
                           } },
                type.mBaseType);
        }

        void generate(std::ostream &stream, const CodeGen::Statement &statement);

        void generate(std::ostream &stream, const Vector4 &v)
        {
            stream << "float4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        }

        void generate(std::ostream &stream, const Vector3 &v)
        {
            stream << "float4(" << v.x << ", " << v.y << ", " << v.z << ")";
        }

        void generate(std::ostream &stream, const Void &v)
        {
            throw 0;
        }

        void generate(std::ostream &stream, const std::string_view &v)
        {
            stream << "\"" << v << "\"";
        }

        void generate(std::ostream &stream, const float &f)
        {
            stream << f;
        }

        template <typename T>
        void generate(std::ostream &stream, const T &v)
        {
            throw 0;
        }

        void generate(std::ostream &stream, const CodeGen::Assignment &statement)
        {
            generate(stream, statement.mTarget);
            stream << " = ";
            generate(stream, statement.mStatement);
        }

        void generate(std::ostream &stream, const CodeGen::Return &statement)
        {
            stream << "return ";
            generate(stream, statement.mStatement);
        }

        void generate(std::ostream &stream, const CodeGen::VariableAccess &statement)
        {
            stream << statement.mVariableName;
        }

        void generate(std::ostream &stream, const CodeGen::VariableDefinition &def)
        {
            generateType(stream, def.mVariable.mType);
            stream << " " << def.mVariable.mName;
            if (def.mInitializer) {
                stream << " = ";
                generate(stream, def.mInitializer);
            }
        }

        void generate(std::ostream &stream, const CodeGen::MemberAccess &access)
        {
            generate(stream, access.mStatement);
            stream << "." << access.mMemberName;
        }

        void generate(std::ostream &stream, const CodeGen::Namespace &ns)
        {
            throw 0;
        }

        void generate(std::ostream &stream, const CodeGen::CustomCodeBlock &block)
        {
            throw 0;
        }

        void generate(std::ostream &stream, const CodeGen::ArithOperation &op)
        {
            const char *opCode;
            switch (op.mType) {
            case CodeGen::ArithOperation::ADD:
                opCode = " + ";
                break;
            case CodeGen::ArithOperation::MUL:
                opCode = " * ";
                break;
            default:
                throw 0;
            }

            bool first = true;
            for (const CodeGen::Statement &statement : op.mOperands) {
                if (first)
                    first = false;
                else
                    stream << opCode;
                generate(stream, statement);
            }

            switch (op.mType) {
            default:
                break;
            }
        }

        void generate(std::ostream &stream, const CodeGen::Constructor &ctor)
        {
            generateType(stream, ctor.mType);
            stream << "(";
            bool first = true;
            for (const CodeGen::Statement &statement : ctor.mArguments) {
                if (first)
                    first = false;
                else
                    stream << ",";
                generate(stream, statement);
            }
            stream << ")";
        }

        void generate(std::ostream &stream, const CodeGen::Constant &c)
        {
            c.mValue.visit([&](const auto &part) { generate(stream, part); });
        }

        void generate(std::ostream &stream, const CodeGen::Statement &statement)
        {
            std::visit([&](const auto &part) { generate(stream, part); }, statement);
        }

        void generate(std::ostream &stream, const CodeGen::ShaderFile &file, ShaderType type)
        {
#if OPENGL_ES
            stream << "#version 300 es\n";
            stream << "precision highp float;\n";
#else
            stream << "#version 410 core\n";
#endif

            for (const std::pair<const std::string, CodeGen::Struct> &structInfo : file.mStructs) {

                if (structInfo.first == "VertexData") {
                    if (type == VertexShader) {
                        uint32_t i = 0;
                        for (const CodeGen::VariableDefinition &arg : structInfo.second.mVariables) {
                            stream << "layout(location = " << i++ << ") in ";
                            generateType(stream, arg.mVariable.mType);
                            stream << " " << arg.mVariable.mName << ";\n";
                        }
                    } else {
                        continue;
                    }
                }

                bool isBuffer = !structInfo.second.mAnnotations.empty() && StringUtil::startsWith(structInfo.second.mAnnotations.front(), "buffer");
                if (isBuffer) {
                    std::string_view number = std::string_view { structInfo.second.mAnnotations.front() }.substr(strlen("buffer"));
                    uint32_t bufferIndex = std::atoi(number.data());
#if !OPENGL_ES
                    stream << "layout (std140, binding = " << bufferIndex << ") ";
#endif
                    stream << "uniform ";
                } else
                    stream << "struct";
                stream << " " << structInfo.second.mName << "{\n";
                for (const CodeGen::VariableDefinition &arg : structInfo.second.mVariables) {
                    stream << "\t";
                    /*for (const std::string &annotation : arg.mAnnotations) {

                        stream << annotation << " ";
                    }*/
                    generateType(stream, arg.mVariable.mType);
                    stream << " " << arg.mVariable.mName << ";\n";
                }
                stream << "};\n\n";
            }

            const std::vector<CodeGen::Function> &functions = *std::next(file.mInstances.begin(), type == VertexShader ? 0 : 1);
            for (const CodeGen::Function &function : functions) {
                std::string_view name = function.mName == "main" ? "mainImpl" : std::string_view { function.mName };
                generateType(stream, function.mReturnType);
                stream << " " << name << "(";
                bool first = true;
                for (const CodeGen::Variable &arg : function.mArguments) {
                    generateType(stream, arg.mType);
                    stream << " " << arg.mName;
                    if (first)
                        first = false;
                    else
                        stream << ",";
                }
                stream << ")";

                stream << "{\n";

                for (const CodeGen::Statement &statement : function.mStatements) {
                    stream << "\t";
                    generate(stream, statement);
                    stream << ";\n";
                }

                stream << "}\n\n";
            }

            stream << (type == VertexShader ? "out" : "in") << " RasterizerData rasterizerData;\n";
            if (type == PixelShader)
                stream << "out vec4 fragColor;\n";
            stream << "\n";
            stream << "void main(){\n";
            if (type == VertexShader) {
                stream << "\tVertexData IN;\n";
                for (const CodeGen::VariableDefinition &inVar : file.mStructs.at("VertexData").mVariables) {
                    stream << "\tIN." << inVar.mVariable.mName << " = " << inVar.mVariable.mName << ";\n";
                }
                stream << "\trasterizerData = mainImpl(IN);\n"
                       << "\tgl_Position = rasterizerData.pos;\n";
            } else {
                stream << "\tfragColor = mainImpl(rasterizerData);\n";
            }
            stream << "}\n";
        }

    }

}
}