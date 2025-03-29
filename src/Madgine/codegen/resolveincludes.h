#pragma once

namespace CodeGen {

    inline void resolveIncludes(std::string &source, std::function<std::string(const Engine::Filesystem::Path &, size_t, std::string_view)> lookup, const std::string_view currentFileName, std::map<std::string, size_t> &files)
    {       

        size_t end = 0;
        size_t line = 1;

        size_t pos = source.find("#include ");
        while (pos != std::string::npos) {
            line += std::count(source.data() + end, source.data() + pos, '\n');
            size_t mark = pos + strlen("#include ");
            assert(source[mark] == '"');
            ++mark;
            end = source.find('"', mark);
            std::string filename { source.begin() + mark, source.begin() + end };
            Engine::Filesystem::Path path { filename };
            ++end;
            std::string code;
            if (files.try_emplace(filename, files.size()).second) {
                code = lookup(path, line, currentFileName);
                resolveIncludes(code, lookup, filename, files);
            }
            source.replace(pos, end - pos, code.c_str());
            end = pos + code.size();
            pos = source.find("#include ", end);
        }
    }

}