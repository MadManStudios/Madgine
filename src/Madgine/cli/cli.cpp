#include "clilib.h"

#include "cli.h"
#include "parameter.h"

namespace Engine {
namespace CLI {

    Parameter<bool> showHelp { { "--help", "-h" }, false, "Show this help message" };

    static CLICore *sSingleton = nullptr;

    CLICore::CLICore(int argc, char **argv)
    {
        assert(!sSingleton);
        sSingleton = this;

        if (argc > 0)
            mProgramPath = argv[0];

        if (argc > 1) {
            char **it = argv + 1;
            char **end = argv + argc;

            if (it != end && **it != '-') {
                LOG_ERROR("Positional arguments are not supported!");
                ++it;
                while (it != end && **it != '-')
                    ++it;
            }

            while (it != end) {
                assert(**it == '-');
                auto pib = mArguments.try_emplace(*it);
                if (!pib.second) {
                    LOG_WARNING("Argument '" << *it << "' provided twice! Last occurence will be used!");
                    pib.first->second.clear();
                }
                std::vector<std::string_view> &args = pib.first->second;
                ++it;
                while (it != end && **it != '-') {
                    args.emplace_back(*it);
                    ++it;
                }
            }

            for (ParameterBase *parameter : parameters())
                parameter->init();

            std::set<std::string_view> unusedArguments;
            //use kvKeys
            for (auto &p : mArguments)
                unusedArguments.insert(p.first);
            for (ParameterBase *parameter : parameters())
                for (const char *option : parameter->options())
                    unusedArguments.erase(option);
            for (std::string_view unusedArgument : unusedArguments)
                LOG_WARNING("Unrecognized command-line argument '" << unusedArgument << "' provided!");
        }

        if (showHelp)
            help();
    }

    CLICore::~CLICore()
    {
        assert(sSingleton == this);
        sSingleton = nullptr;
    }

    void CLICore::help()
    {
        Log::LogDummy out { Log::MessageType::INFO_TYPE };
        out << "Help:\n";
        for (ParameterBase *parameter : parameters()) {
            out << parameter->help() << "\n";
        }
    }

    const CLICore &CLICore::getSingleton()
    {
        assert(sSingleton);
        return *sSingleton;
    }

    std::vector<ParameterBase *> &CLICore::parameters()
    {
        static std::vector<ParameterBase *> dummy;
        return dummy;
    }

	bool CLICore::isInitialized()
    {
        return sSingleton != nullptr;
    }

}
}