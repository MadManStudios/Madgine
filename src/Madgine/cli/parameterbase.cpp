#include "clilib.h"

#include "parameterbase.h"
#include "cli.h"

namespace Engine {
namespace CLI {

    

    ParameterBase::ParameterBase(size_t minArgumentCount, size_t maxArgumentCount, std::vector<const char *> options, const char *help)
        : mOptions(std::move(options))
        , mHelp(help)
        , mMinArgumentCount(minArgumentCount)
        , mMaxArgumentCount(maxArgumentCount)
    {
        CLICore::parameters().emplace_back(this);
    }

    void ParameterBase::init()
    {
        if (!mInitialized) {
            mInitialized = true;

            if (CLICore::isInitialized()) {

                const std::vector<std::string_view> *args = nullptr;
                const char *optionName = nullptr;

                for (const char *option : mOptions) {
                    auto it = CLICore::getSingleton().mArguments.find(option);
                    if (it != CLICore::getSingleton().mArguments.end()) {
                        if (args)
                            LOG_WARNING("Different styles of argument '" << option << "' provided! Which style is used is undefined!");

                        args = &it->second;
                        optionName = option;
                    }
                }

                if (args) {
                    if (args->size() < mMinArgumentCount) {
                        LOG_ERROR("Insufficient amount of arguments provided for option '" << optionName << "'!");
                        if (mHelp)
                            LOG_ERROR("\t" << mHelp);
                        return;
                    }
                    if (args->size() > mMaxArgumentCount) {
                        LOG_WARNING("Too many arguments provided for option '" << optionName << "'! Superfluous arguments will be discarded!");
                    }
                    if (!parse(*args)) {
                        LOG_ERROR("Invalid parameters provided for option '" << optionName << "': " << StringUtil::join(*args, ", "));                        
                        LOG_ERROR("\t" << help());
                    }
                }
            }
        }
    }

    std::string ParameterBase::help()
    {
        std::ostringstream ss;
        ss << StringUtil::join(mOptions, ", ") << ": \t";
        if (mHelp)
            ss << mHelp;
        else
            ss << "<no help provided>";
        ss << " (" << typeName() << ")";
        return ss.str();
    }

    const std::vector<const char *> &ParameterBase::options()
    {
        return mOptions;
    }

}
}