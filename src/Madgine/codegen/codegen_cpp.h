#pragma once

#include "codegen.h"
#include "statement.h"

namespace CodeGen {

struct MADGINE_CODEGEN_EXPORT CppFile : File {

    CppFile();
    CppFile(const CppFile &) = delete;

    CppFile &operator=(const CppFile &) = delete;

    void include(size_t level, std::string_view file);

    void beginNamespace(std::string_view ns);
    void endNamespace();

    virtual void statement(Statement statement) override;

    void generate(std::ostream &stream);

    void generate(std::ostream &stream, const Namespace &ns);
    void generate(std::ostream &stream, const CustomCodeBlock &block);
    void generate(std::ostream &stream, const VariableAccess &access);
    void generate(std::ostream &stream, const VariableDefinition &def);
    void generate(std::ostream &stream, const MemberAccess &access);
    void generate(std::ostream &stream, const Return &ret);
    void generate(std::ostream &stream, const Assignment &assign);
    void generate(std::ostream &stream, const ArithOperation &op);
    void generate(std::ostream &stream, const Constructor &con);
    void generate(std::ostream &stream, const Constant &c);
    void generate(std::ostream &stream, const Comment &c);
    void generate(std::ostream &stream, const ForEach &f);

    void generate(std::ostream &stream, const Statement &s);

    void generate(std::ostream &stream, const Type &t);

private:
    std::vector<std::map<Engine::BitArray<62>, std::set<std::string>>> mIncludes;

    Namespace mGlobalNamespace;
    std::stack<Namespace *> mNamespaceStack;
};

}