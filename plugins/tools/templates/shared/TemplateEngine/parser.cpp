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
        case '%':
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
        case '#':
            if (openTag) {
                hasType = true;
            }
            break;
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
    Engine::ValueType &field = mFields[name];
    Engine::ValueTypeDesc targetType;
    if (type.empty() || type == "lc_string" || type == "uc_string" || type == "string") {
        targetType = Engine::toValueTypeDesc<std::string>();
    } else {
        LOG_ERROR("Unknown type: " << type);
        return;
    }
    if (field.type().mType != Engine::ValueTypeEnum::NullValue && !field.type().canAccept(targetType))
        LOG_ERROR("Incompatible types: " << field.type().toString() << " and " << targetType.toString());
    field.setType(targetType);
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
    std::string type;

    for (char c : std::ranges::subrange { in.iterator(), in.end() }) {
        switch (c) {
        case '%':
            if (!openTag) {
                openTag = true;
                hasType = false;
                name.clear();
            } else {
                std::string value = mFields.at(name).as<std::string>();

                if (hasType) {
                    if (type == "uc_string") {
                        value = Engine::StringUtil::toUpper(std::move(value));
                    } else if (type == "lc_string") {
                        value = Engine::StringUtil::toLower(std::move(value));
                    }
                    hasType = false;
                }
                openTag = false;
                out << value;
            }
            break;
        case '#':
            if (openTag) {
                hasType = true;
                type.clear();
            } else {
                out << c;
            }
            break;
        default:
            if (openTag) {
                if (hasType) {
                    type += c;
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
