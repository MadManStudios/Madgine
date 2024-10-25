#include "codegenlib.h"

#include "codegen_shader.h"

namespace CodeGen {

ShaderFile::ShaderFile()
    : mInstances(1)
    , mCurrentInstance(&mInstances.front())
{
}

ShaderFile::ShaderFile(const ShaderFile &)
{
    throw 0;
}

ShaderFile &ShaderFile::operator=(const ShaderFile &)
{
    throw 0;
}

std::vector<CodeGen::Function> *ShaderFile::nextInstance()
{
    return std::exchange(mCurrentInstance, &mInstances.emplace_back());
}

void ShaderFile::setInstance(std::vector<CodeGen::Function> *instance)
{
    mCurrentInstance = instance;
}

void ShaderFile::beginFunction(std::string_view name, Type returnType, std::vector<Variable> arguments, std::vector<std::string> annotations)
{
    mFunctionStack.push(&mCurrentInstance->emplace_back(name, returnType, std::move(arguments), std::move(annotations)));
}

void ShaderFile::endFunction()
{
    mFunctionStack.pop();
}

void ShaderFile::statement(Statement statement)
{
    mFunctionStack.top()->mStatements.emplace_back(std::move(statement));
}

Struct *ShaderFile::getStruct(std::string_view name)
{
    return &mStructs.try_emplace(std::string { name }, name).first->second;
}

}