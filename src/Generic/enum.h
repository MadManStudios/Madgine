#pragma once

#include "bits/array.h"

namespace Engine {

struct EnumMetaTable {

    constexpr bool isBase(int32_t value) const
    {
        return mBase && value < mBase->mMax;
    }

    constexpr std::string_view toString(int32_t value) const
    {
        if (isBase(value)) {
            return mBase->toString(value);
        } else {
            assert(value > mMin && value < mMax);
            return mValueNames[value - mMin];
        }
    }

    constexpr bool fromString(std::string_view s, int32_t &v) const
    {
        const std::string_view *end = mValueNames + (mMax - mMin);
        auto it = std::find(mValueNames, end, s);
        if (it != end) {
            v = (it - mValueNames) + mMin;
            return true;
        } else if (mBase) {
            return mBase->fromString(s, v);
        } else {
            return false;
        }
    }

    std::ostream &print(std::ostream &stream, int32_t value, std::string_view actualType) const
    {
        if (isBase(value))
            return mBase->print(stream, value, actualType);
        return stream << actualType << "::" << toString(value);
    }

    std::ostream &printFlags(std::ostream &stream, BitArray<64> flags) const
    {
        StringUtil::StreamJoiner join { stream, "|" };
        for (int32_t v : values<int32_t>()) {
            if (flags[v]) {
                print(join.next(), v, mName);
            }
        }
        if (join.empty())
            stream << '0';
        return stream;
    }

    std::istream &read(std::istream &stream, int32_t &value, std::string_view actualType) const
    {
        std::istream::pos_type p = stream.tellg();
        std::string buffer;
        buffer.resize(actualType.size());
        if (!stream.read(buffer.data(), actualType.size())) {
            stream.seekg(p);
            return stream;
        }
        if (actualType != buffer) {
            stream.seekg(p);
            stream.setstate(std::ios_base::failbit);
            return stream;
        }
        if (!stream.read(buffer.data(), 2)) {
            stream.seekg(p);
            return stream;
        }
        if (buffer[0] != ':' || buffer[1] != ':') {
            stream.seekg(p);
            stream.setstate(std::ios_base::failbit);
            return stream;
        }
        if (!(stream >> buffer)) {
            stream.seekg(p);
            return stream;
        }
        if (!fromString(buffer, value)) {
            stream.seekg(p);
            stream.setstate(std::ios_base::failbit);
        }
        return stream;
    }

    std::istream &readFlags(std::istream &stream, BitArray<64> &flags) const
    {
        std::string s;
        stream >> s;
        flags = {};
        if (s != "0") {
            for (std::string_view e : StringUtil::tokenize(s, '|')) {
                if (!e.starts_with(mName))
                    throw 0;
                e.remove_prefix(mName.size());
                if (!e.starts_with("::"))
                    throw 0;
                e.remove_prefix(2);
                int32_t v;
                if (!fromString(e, v))
                    throw 0;
                flags[v] = true;
            }
        }
        return stream;
    }

    template <typename T>
    Generator<T> values() const
    {
        int32_t val = mMin;
        while (val < mMax) {
            co_yield static_cast<T>(val);
            ++val;
        }
    }

    const EnumMetaTable *mBase;
    std::string_view mName;
    const std::string_view *mValueNames;
    int32_t mMin, mMax;
};

template <typename EnumType, typename... Representations>
concept ValidEnumType = OneOf<EnumType, typename Representations::EnumType...>;

template <typename _Representation, typename... Representations>
struct EnumImpl : _Representation, Representations... {

    using Representation = _Representation;
    using underlying_type = typename Representation::underlying_type;

    template <typename DerivedRepresentation>
    using Derived = EnumImpl<DerivedRepresentation, Representation, Representations...>;

    using Representation::MAX;
    using Representation::MIN;
    static constexpr size_t COUNT = MAX - MIN;

    EnumImpl() = default;

    template <ValidEnumType<Representation, Representations...> EnumType>
    EnumImpl(EnumType value)
        : mValue(value)
    {
    }

    explicit EnumImpl(underlying_type intValue)
        : mValue(intValue)
    {
    }

    template <typename T>
        requires std::same_as<EnumImpl<Representations...>, T>
    EnumImpl(T other)
        : mValue(other)
    {
    }

    template <ValidEnumType<Representation, Representations...> EnumType>
    operator EnumType() const
    {
        return static_cast<EnumType>(mValue);
    }

    operator underlying_type() const
    {
        return mValue;
    }

    std::string_view toString() const
    {
        return Representation::sTable.toString(mValue);
    }

    bool fromString(std::string_view s)
    {
        return Representation::sTable.fromString(s, reinterpret_cast<int32_t &>(mValue));
        /*mValue = static_cast<EnumType>(v);
        return true;*/
    }

    template <ValidEnumType<Representation, Representations...> EnumType>
    auto operator<=>(EnumType other) const
    {
        return static_cast<EnumType>(this->mValue) <=> other;
    }

    template <ValidEnumType<Representation, Representations...> EnumType>
    bool operator==(EnumType other) const
    {
        return static_cast<EnumType>(this->mValue) == other;
    }

    template <ValidEnumType<Representation, Representations...> EnumType>
    bool operator!=(EnumType other) const
    {
        return static_cast<EnumType>(this->mValue) != other;
    }

    friend std::ostream &operator<<(std::ostream &stream, const EnumImpl &value)
    {
        return Representation::sTable.print(stream, value.mValue, sTypeName());
    }

    friend std::istream &operator>>(std::istream &stream, EnumImpl &value)
    {
        int32_t dummy;
        if (Representation::sTable.read(stream, dummy, sTypeName()))
            value.mValue = static_cast<underlying_type>(dummy);
        return stream;
    }

    static std::string_view sTypeName()
    {
        return Representation::sTable.mName;
    }

    static Generator<EnumImpl> values()
    {
        return Representation::sTable.template values<EnumImpl>();
    }

protected:
    underlying_type mValue;
};

}

#define ENUM_REGISTRY(Name, Type, MIN_VAL, Base, ...)                                                                                  \
    struct Name##Representation {                                                                                                      \
        enum EnumType Type {                                                                                                           \
            __VA_OPT__(HEAD(__VA_ARGS__) = MIN_VAL,                                                                                    \
                TAIL(__VA_ARGS__))                                                                                                     \
        };                                                                                                                             \
        using underlying_type = std::underlying_type_t<EnumType>;                                                                      \
        static inline const constexpr underlying_type MIN = MIN_VAL;                                                                   \
        static inline const constexpr underlying_type MAX = MIN_VAL + NUM_ARGS(__VA_ARGS__);                                           \
        static inline const constexpr underlying_type COUNT = NUM_ARGS(__VA_ARGS__);                                                   \
        static inline const constexpr auto sIdentifiers = Engine::StringUtil::tokenize<static_cast<size_t>(COUNT)>(#__VA_ARGS__, ','); \
        static inline const constexpr Engine::EnumMetaTable sTable {                                                                   \
            Base, #Name, sIdentifiers.data(), MIN, MAX                                                                                 \
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

#define ENUM_BASE(Name, Base, ...)                                                                      \
    ENUM_REGISTRY(Name, : Base::underlying_type, Base::MAX, &Base##Representation::sTable, __VA_ARGS__) \
    using Name = Base::Derived<Name##Representation>;

#define TYPED_ENUM(Name, Type, ...)                      \
    ENUM_REGISTRY(Name, : Type, 0, nullptr, __VA_ARGS__) \
    using Name = Engine::EnumImpl<Name##Representation>;

#define ENUM(Name, ...)                            \
    ENUM_REGISTRY(Name, , 0, nullptr, __VA_ARGS__) \
    using Name = Engine::EnumImpl<Name##Representation>;

#define FORWARD_ENUM(Name)       \
    struct Name##Representation; \
    using Name = Engine::EnumImpl<Name##Representation>;
