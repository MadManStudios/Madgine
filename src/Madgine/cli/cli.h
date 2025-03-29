#pragma once

#include "Interfaces/filesystem/path.h"

namespace Engine {

namespace CLI {

    struct CLI_EXPORT CLICore {
        CLICore(int argc, char **argv);
        ~CLICore();

        void help();

		static bool isInitialized();
        static const CLICore &getSingleton();
        static std::vector<ParameterBase *> &parameters();

        std::map<std::string_view, std::vector<std::string_view>> mArguments;
        Filesystem::Path mProgramPath;
    };
}
}