#pragma once

#include "Generic/bits/array.h"
#include "Generic/container/tinyvector.h"

namespace CodeGen {

struct MADGINE_CODEGEN_EXPORT File {

    void beginCondition(std::string_view cond);
    void endCondition(std::string_view cond);

    virtual void statement(Statement statement) = 0;

    bool openCStyleGuard(std::ostream &stream, const Engine::TinyVector<Engine::BitArray<62>> &conditionals) const;
    void closeCStyleGuard(std::ostream &stream, bool wasOpened) const;

    
    struct MADGINE_CODEGEN_EXPORT CustomCodeBuilder {
        CustomCodeBuilder(File *file)
            : mFile(file)
        {
        }
        CustomCodeBuilder(CustomCodeBuilder &&) = default;
        ~CustomCodeBuilder();

        CustomCodeBuilder &operator<<(std::string_view code);
        template <typename T>
        requires(!std::convertible_to<T, std::string_view>)
            CustomCodeBuilder &
            operator<<(const T &t)
        {
            return (*this) << std::to_string(t);
        }

        File *mFile;
        std::ostringstream mStream;
    };

    CustomCodeBuilder operator<<(std::string_view code);

    void addCustomCodeBlock(std::string code);


    std::vector<size_t> mConditionalsStack;
    std::vector<std::string> mConditionalTokenList;

    Engine::BitArray<62> mConditionalsBitMask;
};

}