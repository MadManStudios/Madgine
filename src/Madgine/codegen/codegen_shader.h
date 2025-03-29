#pragma once

#include "codegen.h"

#include "statement.h"

namespace CodeGen {

struct MADGINE_CODEGEN_EXPORT ShaderFile : File {

    ShaderFile();
    ShaderFile(const ShaderFile &);
    ShaderFile(ShaderFile &&) = default;

    ShaderFile &operator=(const ShaderFile &);

    std::vector<CodeGen::Function> *nextInstance();
    void setInstance(std::vector<CodeGen::Function> *instance);

    void beginFunction(std::string_view name, Type returnType, std::vector<Variable> arguments, std::vector<std::string> annotations = {});
    void endFunction();

    virtual void statement(Statement statement) override;
    Struct *getStruct(std::string_view name);
    
    std::list<std::vector<Function>> mInstances;
    std::vector<Function> *mCurrentInstance;
    std::stack<Function *> mFunctionStack;
    std::map<std::string, Struct, std::less<>> mStructs;
};

}