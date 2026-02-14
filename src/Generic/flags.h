
#include "bits/array.h"
#include "enum.h"

namespace Engine {

template <typename _Representation>
struct Flags : _Representation {

    using Representation = _Representation;

    Flags() = default;

    Flags(Representation::EnumType bits)
        : mBits(bits)
    {
    }

    Representation::EnumType value() const
    {
        return mBits;
    }

    operator typename Representation::EnumType() const
    {
        return mBits;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Flags<Representation> &value)
    {
        return Representation::sTable.printFlags(stream, value.mBits);
    }

    friend std::istream &operator>>(std::istream &stream, Flags<Representation> &value)
    {
        uint64_t bits;
        Representation::sTable.readFlags(stream, bits);
        value.mBits = static_cast<Representation::EnumType>(bits);
        return stream;
    }

    auto operator<=>(const Flags<Representation> &other) const
    {
        return mBits <=> other.mBits;
    }

    auto operator<=>(Representation::EnumType other) const
    {
        return mBits <=> other;
    }

    bool operator==(Representation::EnumType other) const
    {
        return mBits == other;
    }

private:
    Representation::EnumType mBits = static_cast<Representation::EnumType>(0);
};

}

#define FLAGS_EXPAND(offset, a, ...) a = (1 << (offset)) __VA_OPT__(, FLAGS_EXPAND_HELPER PARENS(offset + 1, __VA_ARGS__))

#define FLAGS_EXPAND_HELPER() FLAGS_EXPAND

#define FLAGS_REGISTRY(Name, ...)                                                                                                      \
    struct Name##Representation {                                                                                                      \
        enum EnumType {                                                                                                                \
            Default = 0,                                                                                                               \
            EVAL2(FLAGS_EXPAND(0, __VA_ARGS__))                                                                                        \
        };                                                                                                                             \
        using underlying_type = std::underlying_type_t<EnumType>;                                                                      \
        static inline const constexpr underlying_type COUNT = NUM_ARGS(__VA_ARGS__);                                                   \
        static inline const constexpr auto sIdentifiers = Engine::StringUtil::tokenize<static_cast<size_t>(COUNT)>(#__VA_ARGS__, ','); \
        static inline const constexpr Engine::EnumMetaTable sTable {                                                                   \
            nullptr, #Name, sIdentifiers.data(), 0, COUNT                                                                              \
        };                                                                                                                             \
    };                                                                                                                                 \
    inline std::ostream &operator<<(std::ostream &stream, typename Name##Representation::EnumType value)                               \
    {                                                                                                                                  \
        return Name##Representation::sTable.print(stream, value, Name##Representation::sTable.mName);                                  \
    }                                                                                                                                  \
    inline std::istream &operator>>(std::istream &stream, typename Name##Representation::EnumType &value)                              \
    {                                                                                                                                  \
        int32_t dummy;                                                                                                                 \
        if (Name##Representation::sTable.read(stream, dummy, Name##Representation::sTable.mName))                                      \
            value = static_cast<typename Name##Representation::EnumType>(dummy);                                                       \
        return stream;                                                                                                                 \
    }

#define FLAGS(Name, ...)              \
    FLAGS_REGISTRY(Name, __VA_ARGS__) \
    using Name = Engine::Flags<Name##Representation>;
