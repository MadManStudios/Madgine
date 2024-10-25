#include "codegenlib.h"

#include "codegen_cpp.h"

#include "statement.h"

namespace CodeGen {

CppFile::CppFile()
    : mNamespaceStack { { &mGlobalNamespace } }
{
}

void CppFile::include(size_t level, std::string_view file)
{
    if (mIncludes.size() <= level)
        mIncludes.resize(level + 1);

    mIncludes[level][{ mConditionalsBitMask }].insert(std::string { file });
}

void CppFile::beginNamespace(std::string_view ns)
{
    mNamespaceStack.push(&std::get<Namespace>(mNamespaceStack.top()->mElements.emplace_back(Namespace {
        { { mConditionalsBitMask } },
        std::string { ns } })));
}

void CppFile::endNamespace()
{
    assert(mNamespaceStack.size() > 1);
    mNamespaceStack.pop();
}

void CppFile::statement(Statement statement)
{
    mNamespaceStack.top()->mElements.emplace_back(std::move(statement));
}

void CppFile::generate(std::ostream &stream)
{
    assert(mNamespaceStack.size() == 1);
    assert(mConditionalsBitMask == Engine::BitArray<62> {});

    for (std::map<Engine::BitArray<62>, std::set<std::string>> &includeMap : mIncludes) {
        for (std::pair<const Engine::BitArray<62>, std::set<std::string>> &p : includeMap) {
            bool hasGuard = openCStyleGuard(stream, { p.first });

            for (const std::string &file : p.second) {
                stream << "#include \"" << file << "\"\n";
            }

            if (hasGuard)
                stream << "#endif\n";
        }
        stream << "\n";
    }

    stream << "\n";

    generate(stream, mGlobalNamespace);
}

void CppFile::generate(std::ostream &stream, const CustomCodeBlock &block)
{
    stream << block.mCode;
}

void CppFile::generate(std::ostream &stream, const Namespace &ns)
{
    if (!ns.mName.empty())
        stream << "namespace " << ns.mName << "{\n";

    for (const Statement &s : ns.mElements) {
        generate(stream, s);
        stream << "\n";
    }

    if (!ns.mName.empty())
        stream << "}\n";
}

void CppFile::generate(std::ostream &stream, const VariableAccess &access)
{
    stream << access.mVariableName;
}

void CppFile::generate(std::ostream &stream, const VariableDefinition &def)
{
    generate(stream, def.mVariable.mType);
    stream << " " << def.mVariable.mName;
    if (def.mInitializer) {
        stream << " = ";
        generate(stream, *def.mInitializer);
    }
    stream << ";";
}

void CppFile::generate(std::ostream &stream, const MemberAccess &access)
{
    throw 0;
}

void CppFile::generate(std::ostream &stream, const Return &ret)
{
    throw 0;
}

void CppFile::generate(std::ostream &stream, const Assignment &assign)
{
    generate(stream, assign.mTarget);
    stream << " = ";
    generate(stream, assign.mStatement);
    stream << ";";
}

void CppFile::generate(std::ostream &stream, const ArithOperation &op)
{
    bool first = true;
    for (const Statement &ops : op.mOperands) {
        if (first)
            first = false;
        else
            stream << " + ";
        generate(stream, ops);
    }
}

void CppFile::generate(std::ostream &stream, const Constructor &con)
{
    throw 0;
}

DERIVE_OPERATOR(StreamOut, <<)

void CppFile::generate(std::ostream &stream, const Constant &c)
{
    c.mValue.visit([&](const auto &v) {
        if constexpr (has_operator_StreamOut<decltype(v), std::ostream &> || has_operator_StreamOut<std::ostream &, decltype(v)>)
            stream << std::forward<decltype(v)>(v);
    });
}

void CppFile::generate(std::ostream &stream, const Comment &c)
{
    throw 0;
}

void CppFile::generate(std::ostream &stream, const ForEach &f)
{
    stream << "for(auto && " << f.mVariableName << " : ";
    generate(stream, f.mInput);
    stream << ") {\n";
    for (const Statement &s : f.mBody) {
        stream << "\t";
        generate(stream, s);
        stream << "\n";
    }
    stream << "}";
}

void CppFile::generate(std::ostream &stream, const Statement &s)
{

    std::visit([&](auto &block) {
        bool hasGuard;
        if constexpr (hasConditionals<decltype(block)>)
            hasGuard = openCStyleGuard(stream, block.mConditionals);

        generate(stream, block);

        if constexpr (hasConditionals<decltype(block)>)
            closeCStyleGuard(stream, hasGuard);
    },
        s);
}

void CppFile::generate(std::ostream &stream, const Type &t)
{
    std::visit(Engine::overloaded { [&](Struct *s) {
                                       stream << s->mName;
                                   },
                   [&](const Engine::ExtendedValueTypeDesc &desc) {
                       if (desc.mType.isRegular())
                           stream << static_cast<Engine::ValueTypeDesc>(desc).toTypeName();
                       else
                           stream << "auto";
                   } },
        t.mBaseType);
    if (t.mIsReference)
        stream << "&";
}

}