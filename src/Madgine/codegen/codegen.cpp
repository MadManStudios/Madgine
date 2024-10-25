#include "codegenlib.h"

#include "codegen.h"

#include "statement.h"

namespace CodeGen {

void File::beginCondition(std::string_view cond)
{
    auto it = std::ranges::find(mConditionalTokenList, cond);
    if (it == mConditionalTokenList.end()) {
        mConditionalTokenList.emplace_back(cond);
        it = std::prev(mConditionalTokenList.end());
        assert(mConditionalsStack.size() == std::distance(mConditionalTokenList.begin(), it));
        mConditionalsStack.emplace_back();
    }
    size_t index = std::distance(mConditionalTokenList.begin(), it);
    ++mConditionalsStack[index];
    if (mConditionalsStack[index] == 1)
        mConditionalsBitMask[index] = true;
}

void File::endCondition(std::string_view cond)
{
    auto it = std::ranges::find(mConditionalTokenList, cond);
    assert(it != mConditionalTokenList.end());
    size_t index = std::distance(mConditionalTokenList.begin(), it);
    --mConditionalsStack[index];
    if (mConditionalsStack[index] == 0)
        mConditionalsBitMask[index] = false;
}

bool File::openCStyleGuard(std::ostream &stream, const Engine::TinyVector<Engine::BitArray<62>> &conditionals) const
{
    if (std::ranges::all_of(conditionals, [](const Engine::BitArray<62> &cond) { return cond == Engine::BitArray<62> {}; }))
        return false;

    stream << "#if ";

    bool outerFirst = true;

    for (Engine::BitArray<62> cond : conditionals) {
        if (outerFirst)
            outerFirst = false;
        else
            stream << " || ";

        if (cond == Engine::BitArray<62> {}) {
            stream << "1";
            continue;
        }

        stream << "(";

        bool first = true;

        for (size_t i = 0; i < 62; ++i) {
            if (cond[i]) {
                if (first)
                    first = false;
                else
                    stream << " && ";
                stream << "defined(" << mConditionalTokenList[i] << ")";
            }
        }

        stream << ")";
    }

    stream << "\n";
    return true;
}

void File::closeCStyleGuard(std::ostream &stream, bool wasOpened) const
{
    if (wasOpened)
        stream << "#endif\n";
}

File::CustomCodeBuilder::~CustomCodeBuilder()
{
    mFile->addCustomCodeBlock(mStream.str());
}

File::CustomCodeBuilder &File::CustomCodeBuilder::operator<<(std::string_view code)
{
    mStream << code;
    return *this;
}

File::CustomCodeBuilder File::operator<<(std::string_view code)
{
    CustomCodeBuilder builder { this };
    builder.mStream << code;
    return builder;
}

void File::addCustomCodeBlock(std::string code)
{
    statement(CustomCodeBlock { { { mConditionalsBitMask } },
        std::move(code) });
}

}