#pragma once

#include "Interfaces/filesystem/path.h"

#include "Meta/keyvalue/valuetype.h"

namespace TemplateEngine {

struct MADGINE_TEMPLATE_ENGINE_EXPORT Parser {
    Parser(Engine::Filesystem::Path source);

    void parse(std::string_view s);
    void parse(Engine::Stream &s);

    void generateFiles(const Engine::Filesystem::Path &target) const;

    void registerField(std::string name, std::string type);

    std::string generate(std::string_view s) const;
    void generate(Engine::Stream &in, Engine::Stream &out) const;

    std::map<std::string, Engine::ValueType> &fields();

private:
    Engine::Filesystem::Path mSource;
    std::map<std::string, Engine::ValueType> mFields;
};

}