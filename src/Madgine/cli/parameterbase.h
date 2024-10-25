#pragma once

namespace Engine {

namespace CLI {

    struct CLI_EXPORT ParameterBase {

        ParameterBase(size_t minArgumentCount, size_t maxArgumentCount, std::vector<const char *> options, const char *help = nullptr);

        virtual bool parse(const std::vector<std::string_view> &args) = 0;
        virtual const char *typeName() = 0;

        void init();
        virtual std::string help();
        const std::vector<const char *> &options();

    private:
        std::vector<const char *> mOptions;
        const char *mHelp = nullptr;
        size_t mMinArgumentCount, mMaxArgumentCount;

        bool mInitialized = false;
    };

}
}