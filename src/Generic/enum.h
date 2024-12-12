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
            return mValueNames[value - mMin - 1];
        }
    }

    constexpr bool fromString(std::string_view s, int32_t &v) const
    {
        const std::string_view *end = mValueNames + (mMax - mMin - 1);
        auto it = std::find(mValueNames, end, s);
        if (it != end) {
            v = (it - mValueNames) + mMin + 1;
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
        bool first = true;
        for (int32_t v : values<int32_t>()) {
            if (flags[v]) {
                if (first)
                    first = false;
                else
                    stream << '|';
                print(stream, v, mName);
            }
        }
        if (first)
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
        int32_t val = mMin + 1;
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

template <typename _Representation>
struct EnumType : _Representation {

    using Representation = _Representation;
    using BaseType = typename Representation::EnumType;

    EnumType() = default;

    EnumType(BaseType value)
        : mValue(value)
    {
    }

    explicit EnumType(typename Representation::underlying_type intValue)
        : mValue(static_cast<BaseType>(intValue))
    {
    }

    operator BaseType() const
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

    friend std::ostream &operator<<(std::ostream &stream, const EnumType<Representation> &value)
    {
        return Representation::sTable.print(stream, value.mValue, sTypeName());
    }

    friend std::istream &operator>>(std::istream &stream, EnumType<Representation> &value)
    {
        int32_t dummy;
        if (Representation::sTable.read(stream, dummy, sTypeName()))
            value.mValue = static_cast<BaseType>(dummy);
        return stream;
    }

    static std::string_view sTypeName()
    {
        return Representation::sTable.mName;
    }

    static Generator<BaseType> values()
    {
        return Representation::sTable.template values<BaseType>();
    }

protected:
    BaseType mValue;
};

template <typename Base, typename _Representation>
struct BaseEnum : Base, _Representation {
public:
    using Representation = _Representation;
    using BaseType = typename Representation::EnumType;
    using underlying_type = typename Base::underlying_type;
    

    static constexpr int MIN = Base::MIN;
    static constexpr int MAX = BaseType::MAX;
    static constexpr int COUNT = MAX - MIN - 1;

    using Base::Base;

    BaseEnum(BaseType base)
        : Base(static_cast<typename Base::BaseType>(base))
    {
    }

    BaseEnum(Base base)
        : Base(base)
    {
    }

    std::string_view toString() const
    {
        return Representation::sTable.toString(this->mValue);
    }

    friend std::ostream &operator<<(std::ostream &stream, const BaseEnum<Base, _Representation> &value)
    {
        return Representation::sTable.print(stream, value.mValue, sTypeName());
    }

    friend std::istream &operator>>(std::istream &stream, BaseEnum<Base, _Representation> &value)
    {
        return Representation::sTable.read(stream, value.mValue, sTypeName());
    }

    static std::string_view sTypeName()
    {
        return Representation::sTable.mName;
    }

private:
    Base toBase() const
    {
        return { static_cast<typename Base::BaseType>(this->mValue) };
    }
};

}

#define ENUM_REGISTRY(Name, Type, MIN_VAL, Base, ...)                                                                                                        \
    struct Name##Representation {                                                                                                                            \
        enum EnumType Type {                                                                                                                                 \
            MIN = MIN_VAL __VA_OPT__(,)                                                                                                                                   \
            __VA_ARGS__,                                                                                                                                      \
            MAX,                                                                                                                                             \
            COUNT = MAX - MIN - 1                                                                                                                            \
        };                                                                                                                                                   \
        using underlying_type = std::underlying_type_t<EnumType>;                                                                                              \
        static inline const constexpr auto sIdentifiers = Engine::StringUtil::tokenize<static_cast<size_t>(Name##Representation::COUNT)>(#__VA_ARGS__, ','); \
        static inline const constexpr Engine::EnumMetaTable sTable {                                                                                         \
            Base, #Name, sIdentifiers.data(), MIN, MAX                                                                                                       \
        };                                                                                                                                                   \
    };                                                                                                                                                       \
    inline std::ostream &operator<<(std::ostream &stream, typename Name##Representation::EnumType value)                                                     \
    {                                                                                                                                                        \
        return Name##Representation::sTable.print(stream, value, Name##Representation::sTable.mName);                                                        \
    }                                                                                                                                                        \
    inline std::istream &operator>>(std::istream &stream, typename Name##Representation::EnumType &value)                                                    \
    {                                                                                                                                                        \
        int32_t dummy;                                                                                                                                       \
        if (Name##Representation::sTable.read(stream, dummy, Name##Representation::sTable.mName))                                                            \
            value = static_cast<typename Name##Representation::EnumType>(dummy);                                                                             \
        return stream;                                                                                                                                       \
    }

#define ENUM_BASE(Name, Base, ...)                                                                        \
    ENUM_REGISTRY(Name, : Base::underlying_type, Base::MAX - 1, &Base##Representation::sTable, __VA_ARGS__) \
    using Name = Engine::BaseEnum<Base, Name##Representation>;

#define TYPED_ENUM(Name, Type, ...)                 \
    ENUM_REGISTRY(Name,                             \
                  : Type, -1, nullptr, __VA_ARGS__) \
    using Name = Engine::EnumType<Name##Representation>;

#define ENUM(Name, ...)                             \
    ENUM_REGISTRY(Name, , -1, nullptr, __VA_ARGS__) \
    using Name = Engine::EnumType<Name##Representation>;

#define FORWARD_ENUM(Name)       \
    struct Name##Representation; \
    using Name = Engine::EnumType<Name##Representation>;
