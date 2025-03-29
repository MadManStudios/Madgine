#pragma once

#pragma once

#include "codegen.h"
#include "statement.h"

namespace CodeGen {

struct MADGINE_CODEGEN_EXPORT CMakeFile : File {

    CMakeFile();
    CMakeFile(const CMakeFile &) = delete;

    CMakeFile &operator=(const CMakeFile &) = delete;

    void setVariable(std::string_view name, std::string_view string);

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
    void generate(std::ostream &stream, const ForEach &c);

    void generate(std::ostream &stream, const Statement &s);

private:
    std::vector<Statement> mStatements;
};

}