#pragma once

#include "Generic/bits/array.h"
#include "Generic/container/tinyvector.h"
#include "Meta/keyvalue/valuetype.h"

namespace CodeGen {

struct Type {
    std::variant<Engine::ExtendedValueTypeDesc, Struct *> mBaseType;
    bool mIsReference = false;
};

struct MADGINE_CODEGEN_EXPORT StatementVector {
    StatementVector();
    StatementVector(std::initializer_list<Statement> statements);
    StatementVector(const StatementVector &);
    ~StatementVector();

    StatementVector &operator=(const StatementVector &);

    std::vector<Statement>::const_iterator begin() const;
    std::vector<Statement>::const_iterator end() const;

private:
    std::vector<Statement> mVector;
};

struct MADGINE_CODEGEN_EXPORT StatementPtr {
    template <typename T>
    StatementPtr(T t);
    StatementPtr(Statement statement);
    StatementPtr(const StatementPtr &other);

    StatementPtr &operator=(const StatementPtr &other);

    operator const Statement &() const;

    std::unique_ptr<Statement> mPtr;
};

struct FullStatement {
    Engine::TinyVector<Engine::BitArray<62>> mConditionals;
};

template <typename T>
concept hasConditionals = requires(T &t)
{
    t.mConditionals;
};

struct Variable {
    std::string mName;
    Type mType;
    std::vector<std::string> mAnnotations;
};

struct CustomCodeBlock : FullStatement {
    std::string mCode;
};

struct Namespace : FullStatement {
    std::string mName;
    std::list<Statement> mElements;
};

struct Assignment : FullStatement {

    StatementPtr mTarget;
    StatementPtr mStatement;
};

struct Return : FullStatement {

    StatementPtr mStatement;
};

struct VariableAccess {
    std::string mVariableName;
};

struct MADGINE_CODEGEN_EXPORT VariableDefinition : FullStatement {
    VariableDefinition(Variable variable);
    VariableDefinition(Variable variable, Statement initializer);
    VariableDefinition(Engine::TinyVector<Engine::BitArray<62>> conditionals, Variable variable);
    VariableDefinition(const VariableDefinition &other);

    VariableDefinition &operator=(const VariableDefinition &other);

    Variable mVariable;
    std::unique_ptr<Statement> mInitializer;
};

struct MemberAccess {
    std::string mMemberName;
    StatementPtr mStatement;
};

struct Constructor : FullStatement {
    Type mType;
    StatementVector mArguments;
};

struct Constant {
    Engine::ValueType mValue;
};

struct Comment {
    std::string mText;
};

struct ArithOperation {

    enum {
        ADD,
        MUL,
        DIV
    } mType;
    StatementVector mOperands;
};

struct MADGINE_CODEGEN_EXPORT Function : FullStatement {
    Function(std::string_view name, Type returnType, std::vector<Variable> arguments, std::vector<std::string> annotations);
    Function(const Function &) = delete;
    Function(Function &&) = default;

    std::string mName;
    Type mReturnType;
    std::vector<Variable> mArguments;
    std::list<Statement> mStatements;
    std::vector<std::string> mAnnotations;
};

struct ForEach : FullStatement {
    StatementPtr mInput;
    std::string mVariableName;
    std::list<Statement> mBody;
};

struct Struct : FullStatement {
    Struct(std::string_view name);

    std::string mName;
    std::vector<VariableDefinition> mVariables;
    std::vector<std::string> mAnnotations;
};

/* template <typename T>
requires(!std::is_constructible_v<Statement, T>)
    ArithOperation
    operator+(const Statement &first, const T &second)
{
    throw 0;
}*/

inline ArithOperation operator+(const Statement &first, const Statement &second)
{
    return ArithOperation { ArithOperation::ADD, { first, second } };
}

inline ArithOperation operator/(const Statement &first, const Statement &second)
{
    return ArithOperation { ArithOperation::DIV, { first, second } };
}

template <typename T>
inline StatementPtr::StatementPtr(T t)
    : StatementPtr(Statement { t })
{
}

}