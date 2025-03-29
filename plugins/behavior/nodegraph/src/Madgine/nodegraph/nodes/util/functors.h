#pragma once

#include "Meta/math/vector4.h"

namespace Engine {
namespace NodeGraph {

    struct Add {
        auto operator()(const auto &...v) const { return (v + ...); };
    };

    struct Divide {
        auto operator()(const auto &...v) const { return (v / ...); };
    };

    struct Log {
        void operator()(const auto &...v) const
        {
            (Engine::Log::LogDummy { Engine::Log::MessageType::INFO_TYPE } << ... << v);
        }
        CodeGen::Statement operator()(const CodeGen::Statement &s) const
        {
            return s;
        }
    };

    struct Vector3To4 {
        Vector4 operator()(Vector3 v, float w) const
        {
            return { v, w };
        }
        CodeGen::Statement operator()(const CodeGen::Statement &v, const CodeGen::Statement &w) const
        {
            return CodeGen::Constructor { {}, { toValueTypeDesc<Vector4>() }, { v, w } };
        }
    };

    struct BreakVector3 {
        std::tuple<float, float, float> operator()(Vector3 v) const
        {
            return { v.x, v.y, v.z };
        }
        std::tuple<CodeGen::Statement, CodeGen::Statement, CodeGen::Statement> operator()(const CodeGen::Statement &v) const
        {
            return { CodeGen::MemberAccess { "x", v }, CodeGen::MemberAccess { "y", v }, CodeGen::MemberAccess { "z", v } };
        };
    };

    struct BreakVector4 {
        std::tuple<float, float, float, float> operator()(Vector4 v) const
        {
            return { v.x, v.y, v.z, v.w };
        }
        std::tuple<CodeGen::Statement, CodeGen::Statement, CodeGen::Statement, CodeGen::Statement> operator()(const CodeGen::Statement &v) const
        {
            return { CodeGen::MemberAccess { "x", v }, CodeGen::MemberAccess { "y", v }, CodeGen::MemberAccess { "z", v }, CodeGen::MemberAccess { "w", v } };
        };
    };

    struct MakeVector3 {
        Vector3 operator()(float x, float y, float z) const
        {
            return { x, y, z };
        }
        CodeGen::Statement operator()(const CodeGen::Statement &x, const CodeGen::Statement &y, const CodeGen::Statement &z) const
        {
            return CodeGen::Constructor { {}, { toValueTypeDesc<Vector3>() }, { x, y, z } };
        };
    };

}
}