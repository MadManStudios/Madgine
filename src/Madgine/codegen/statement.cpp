#include "codegenlib.h"

#include "statement.h"

namespace CodeGen {

Function::Function(std::string_view name, Type returnType, std::vector<Variable> arguments, std::vector<std::string> annotations)
    : mName(name)
    , mReturnType(returnType)
    , mArguments(std::move(arguments))
    , mAnnotations(std::move(annotations))
{
}

Struct::Struct(std::string_view name)
    : mName(name)
{
}

StatementVector::StatementVector() = default;
StatementVector::StatementVector(std::initializer_list<Statement> statements)
    : mVector(std::move(statements))
{
}
StatementVector::StatementVector(const StatementVector &) = default;
StatementVector::~StatementVector() = default;
StatementVector &StatementVector::operator=(const StatementVector &) = default;

std::vector<Statement>::const_iterator StatementVector::begin() const
{
    return mVector.begin();
}

std::vector<Statement>::const_iterator StatementVector::end() const
{
    return mVector.end();
}

VariableDefinition::VariableDefinition(Variable variable)
    : mVariable(variable)
{
}

VariableDefinition::VariableDefinition(Variable variable, Statement initializer)
    : mVariable(variable)
    , mInitializer(std::make_unique<Statement>(initializer))
{
}

VariableDefinition::VariableDefinition(Engine::TinyVector<Engine::BitArray<62>> conditionals, Variable variable)
    : FullStatement { conditionals }
    , mVariable(variable)
{
}

VariableDefinition::VariableDefinition(const VariableDefinition &other)
    : mVariable(other.mVariable)
    , mInitializer(other.mInitializer ? std::make_unique<Statement>(*other.mInitializer) : std::unique_ptr<Statement> {})
{
}

VariableDefinition &VariableDefinition::operator=(const VariableDefinition &other)
{
    mVariable = other.mVariable;
    if (!other.mInitializer) {
        mInitializer.reset();
    } else {
        if (!mInitializer) {
            mInitializer = std::make_unique<Statement>(*other.mInitializer);
        } else {
            *mInitializer = *other.mInitializer;
        }
    }
    return *this;
}

StatementPtr::StatementPtr(Statement statement)
    : mPtr(std::make_unique<Statement>(statement))
{
}

StatementPtr::StatementPtr(const StatementPtr &other)
    : mPtr(std::make_unique<Statement>(*other.mPtr))
{
}

StatementPtr &StatementPtr::operator=(const StatementPtr &other)
{
    *mPtr = *other.mPtr;
    return *this;
}

StatementPtr::operator const Statement &() const
{
    return *mPtr;
}

}