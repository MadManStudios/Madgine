#pragma once

namespace Engine {
namespace Reflect {

    struct META_EXPORT Enum {

        template <typename Rep, typename... Reps>
        Enum(EnumImpl<Rep, Reps...> e)
            : mValue(static_cast<int32_t>(e))
            , mTable(&Rep::sTable)
        {
        }

        Enum(const EnumMetaTable *table, int32_t value = 0)
            : mValue(value)
            , mTable(table)
        {
        }

        bool operator==(const Enum &other) const;

        template <typename Rep, typename... Reps>
        bool operator!=(const EnumImpl<Rep, Reps...> &e) const
        {
            if (mTable != &Rep::sTable)
                return true;
            return static_cast<typename Rep::EnumType>(mValue) != e;
        }

        std::string_view toString() const;
        const EnumMetaTable *table() const;

        int32_t value() const;
        void setValue(int32_t val);

        template <typename T>
        T safe_cast() const
        {
            if (mTable != &T::Representation::sTable)
                throw 0;
            return static_cast<typename T::Representation::EnumType>(mValue);
        }

        META_EXPORT friend std::ostream &operator<<(std::ostream &stream, const Enum &value);
        META_EXPORT friend std::istream &operator>>(std::istream &stream, Enum &value);

    private:
        int32_t mValue;
        const EnumMetaTable *mTable;
    };

}
}