#include "codegenlib.h"

#include "codegen_cmake.h"

#include "statement.h"

#include "Meta/keyvalue/valuetype.h"

#include "Meta/serialize/streams/formatter.h"

namespace CodeGen {

static std::array<std::ctype<char>::mask, 256> sBracket = Engine::Serialize::generateMask(~Engine::Serialize::ctype::space, { { '(', Engine::Serialize::ctype::space, ~0 }, { ')', Engine::Serialize::ctype::space, ~0 } });
static std::locale sLocaleBracket { std::locale {}, new Engine::Serialize::ctype { sBracket.data() } };

CMakeFile::CMakeFile()
{
}

void CMakeFile::setVariable(std::string_view name, std::string_view string)
{
    statement(Assignment { {} , VariableAccess { std::string { name } }, Constant { Engine::ValueType { string } } });
}

void CMakeFile::statement(Statement statement)
{
    mStatements.emplace_back(std::move(statement));
}

void CMakeFile::generate(std::ostream &stream)
{
    assert(mConditionalsBitMask == Engine::BitArray<62> {});

    for (Statement &statement : mStatements) {
        generate(stream, statement);
        stream << '\n';
    }
}

void CMakeFile::generate(std::ostream &stream, const CustomCodeBlock &block)
{
    stream << block.mCode;
}

void CMakeFile::generate(std::ostream &stream, const Namespace &ns)
{
    throw 0;
}

void CMakeFile::generate(std::ostream &stream, const VariableAccess &access)
{
    throw 0;
}

void CMakeFile::generate(std::ostream &stream, const VariableDefinition &def)
{
    throw 0;
}

void CMakeFile::generate(std::ostream &stream, const MemberAccess &access)
{
    throw 0;
}

void CMakeFile::generate(std::ostream &stream, const Return &ret)
{
    throw 0;
}

void CMakeFile::generate(std::ostream &stream, const Assignment &assign)
{
    stream << "set(";
    generate(stream, assign.mTarget);
    stream << " ";
    generate(stream, assign.mStatement);
    stream << ")";
}

void CMakeFile::generate(std::ostream &stream, const ArithOperation &op)
{
    throw 0;
}

void CMakeFile::generate(std::ostream &stream, const Constructor &con)
{
    throw 0;
}

void CMakeFile::generate(std::ostream &stream, const Constant &c)
{
    stream << c.mValue.toShortString();
}

void CMakeFile::generate(std::ostream &stream, const Comment &c)
{
    throw 0;
}

void CMakeFile::generate(std::ostream &stream, const ForEach &c)
{
    throw 0;
}

void CMakeFile::generate(std::ostream &stream, const Statement &s)
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

}