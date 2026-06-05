#pragma once

#include "Platform/filesystem/path.h"

namespace TemplateEngine {

struct MADGINE_TEMPLATE_ENGINE_EXPORT Parser {
    Parser(Engine::Platform::Filesystem::Path source);

    void parse(std::string_view s);
    void parse(Engine::Stream &s);

    void generateFiles(const Engine::Platform::Filesystem::Path &target) const;

    void registerField(std::string name, std::string type);

    std::string generate(std::string_view s) const;
    void generate(Engine::Stream &in, Engine::Stream &out) const;

    std::map<std::string, Engine::Reflect::Value> &fields();

private:
    Engine::Platform::Filesystem::Path mSource;
    std::map<std::string, Engine::Reflect::Value> mFields;
};

}