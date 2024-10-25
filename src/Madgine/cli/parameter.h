#pragma once

#include "parameterbase.h"


#include "Interfaces/filesystem/path.h"

namespace Engine {

namespace CLI {

    template <typename T>
    struct ParameterImpl : ParameterBase {

        ParameterImpl(std::vector<const char *> options, T defaultValue = {}, const char *help = nullptr)
            : ParameterBase(std::same_as<T, bool> ? 0 : 1, 1, std::move(options), help)
            , mValue(std::move(defaultValue))
        {
        }

        bool parse(const std::vector<std::string_view> &args) override;

        const char *typeName() override
        {
            return std::same_as<T, std::string> ? "string" : typeid(T).name();
        }

        const T &operator*()
        {
            init();
            return mValue;
        }

        const T *operator->()
        {
            init();
            return &mValue;
        }

        operator const T &()
        {
            init();
            return mValue;
        }

    private:
        T mValue;
    };

    template <>
    inline bool ParameterImpl<bool>::parse(const std::vector<std::string_view> &args)
    {
        if (args.empty())
            mValue = true;
        else {
            std::string_view arg = args.front();
            if (arg == "on" || arg == "true")
                mValue = true;
            else if (arg == "off" || arg == "false")
                mValue = false;
            else
                return false;
        }
        return true;
    }

    template <>
    inline bool ParameterImpl<int>::parse(const std::vector<std::string_view> &args)
    {
        errno = 0;
        char *end;
        int value = strtol(args.front().data(), &end, 0);
        if (errno == 0 && *end == '\0') {
            mValue = value;
            return true;
        } else {
            return false;
        }
    }

    template <>
    inline bool ParameterImpl<std::string>::parse(const std::vector<std::string_view> &args)
    {
        mValue = args.front();
        return true;
    }

    template <>
    inline bool ParameterImpl<Filesystem::Path>::parse(const std::vector<std::string_view> &args)
    {
        mValue = args.front();
        return true;
    }

    template <typename T>
    struct Parameter : ParameterImpl<T> {
        Parameter(std::vector<const char *> options, T defaultValue, const char *help)
            : ParameterImpl<T>(std::move(options), std::move(defaultValue), help)
        {
        }
    };

    template <>
    struct Parameter<std::string> : ParameterImpl<std::string> {
        //using ParameterImpl<std::string>::ParameterImpl;
        Parameter(std::vector<const char *> options, std::string defaultValue, const char *help)
            : ParameterImpl(std::move(options), std::move(defaultValue), help)
        {
        }

        operator const char *()
        {
            return static_cast<const std::string &>(*this).c_str();
        }

        operator std::string_view()
        {
            return static_cast<const std::string &>(*this);
        }
    };

    template <typename Rep>
    struct ParameterImpl<EnumType<Rep>> : ParameterBase {
        ParameterImpl(std::vector<const char *> options, EnumType<Rep> defaultValue, const char *help = nullptr)
            : ParameterBase(1, 1, std::move(options), help)
            , mValue(defaultValue)
        {
        }

        bool parse(const std::vector<std::string_view> &args) override
        {
            return mValue.fromString(args[0]);
        }

        const char *typeName() override
        {
            return EnumType<Rep>::sTypeName().data();
        }

        std::string help() override
        {
            std::ostringstream ss;
            ss << ParameterBase::help();
            ss << " [";
            bool first = true;
            for (int i = EnumType<Rep>::MIN + 1; i < EnumType<Rep>::MAX; ++i) {
                if (first)
                    first = false;
                else
                    ss << ", ";
                ss << Rep::sTable.toString(i);
            }
            ss << "].";
            return ss.str();
        }

        const EnumType<Rep> &operator*()
        {
            init();
            return mValue;
        }

        const EnumType<Rep> *operator->()
        {
            init();
            return &mValue;
        }

        operator const EnumType<Rep> &()
        {
            init();
            return mValue;
        }

    private:
        EnumType<Rep> mValue;
    };

}
}