#include "templatelib.h"

#include "parser.h"

#include "Interfaces/filesystem/fsapi.h"

namespace TemplateEngine {

Parser::Parser(Engine::Filesystem::Path source)
    : mSource(source)
{
    if (!Engine::Filesystem::isDir(source))
        throw 0;

    for (Engine::Filesystem::FileQueryResult fileOrDir : Engine::Filesystem::listFilesAndDirsRecursive(source)) {
        parse(fileOrDir.path().filename().str());
        if (!fileOrDir.isDir()) {
            Engine::Stream contentStream = Engine::Filesystem::openFileRead(fileOrDir);
            parse(contentStream);
        }
    }
}

void Parser::parse(std::string_view s)
{
    Engine::Stream stream { std::make_unique<std::stringbuf>(std::string { s }) };
    parse(stream);
}

void Parser::parse(Engine::Stream &s)
{
    bool openTag = false;
    bool hasType = false;
    std::string name;
    std::string type;

    for (char c : std::ranges::subrange { s.iterator(), s.end() }) {
        switch (c) {
        case '$':
            if (!openTag) {
                openTag = true;
                hasType = false;
                name.clear();
                type.clear();
            } else {
                openTag = false;
                hasType = false;
                registerField(std::move(name), std::move(type));
            }
            break;
        case ':':
            if (openTag) {
                hasType = true;
            }
        default:
            if (openTag) {
                if (hasType) {
                    type += c;
                } else {
                    name += c;
                }
            }
        }
    }
}

void Parser::generateFiles(const Engine::Filesystem::Path &target) const
{
    for (Engine::Filesystem::FileQueryResult fileOrDir : Engine::Filesystem::listFilesAndDirsRecursive(mSource)) {

        Engine::Filesystem::Path path = target / fileOrDir.path().parentPath().relative(mSource) / generate(fileOrDir.path().filename().str());

        if (fileOrDir.isDir()) {
            Engine::Filesystem::createDirectories(path);
        } else {
            Engine::Stream contentStream = Engine::Filesystem::openFileRead(fileOrDir);
            Engine::Stream outStream = Engine::Filesystem::openFileWrite(path);
            generate(contentStream, outStream);
        }
    }
}

void Parser::registerField(std::string name, std::string type)
{
    mFields[name].setType(Engine::toValueTypeDesc<std::string>());
}

std::string Parser::generate(std::string_view s) const
{
    Engine::Stream in { std::make_unique<std::stringbuf>(std::string { s }) };
    std::unique_ptr<std::stringbuf> outBuf = std::make_unique<std::stringbuf>();
    std::stringbuf *outRef = outBuf.get();
    Engine::Stream out { std::move(outBuf) };
    generate(in, out);
    return outRef->str();
}

void Parser::generate(Engine::Stream &in, Engine::Stream &out) const
{
    bool openTag = false;
    bool hasType = false;
    std::string name;

    for (char c : std::ranges::subrange { in.iterator(), in.end() }) {
        switch (c) {
        case '$':
            if (!openTag) {
                openTag = true;
                hasType = false;
                name.clear();
            } else {
                openTag = false;
                hasType = false;
                out << mFields.at(name).as<std::string>();
            }
            break;
        case ':':
            if (openTag) {
                hasType = true;
            }
        default:
            if (openTag) {
                if (hasType) {
                } else {
                    name += c;
                }
            } else {
                out << c;
            }
        }
    }
}

std::map<std::string, Engine::ValueType> &Parser::fields()
{
    return mFields;
}

}
