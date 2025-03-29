#pragma once

namespace Engine {

struct META_EXPORT ApiFunction {    

    constexpr ApiFunction() = default;

    constexpr ApiFunction(const FunctionTable *table)
        : mTable(table)
    {
    }

    bool operator==(const ApiFunction &other) const
    {
        return mTable == other.mTable;
    }

    explicit operator bool() const
    {
        return mTable != nullptr;
    }

    void operator()(ValueType &retVal, const ArgumentList &args) const;

    size_t argumentsCount(bool excludeThis = false) const;
    bool isMemberFunction() const;
    
    const FunctionTable *mTable = nullptr;
};

template <auto f>
struct TypedApiFunction {
};

}