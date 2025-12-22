#pragma once

#include "binding.h"

namespace Engine {
namespace Execution {

    template <typename T>
    struct Bindable {

        Bindable() = default;

        template <std::convertible_to<T> U>
        Bindable(U &&value)
            : mValue(static_cast<T>(std::forward<U>(value)))
        {
        }

        template <Binding<T> U>
        Bindable(U &&binding)
            : mValue(Execution::BindingPtr<T> { std::forward<U>(binding) })
        {
        }

        template <std::convertible_to<T> U>
        Bindable &operator=(U &&value)
        {
            mValue = static_cast<T>(std::forward<U>(value));
            return *this;
        }

        template <Binding<T> U>
        Bindable &operator=(U &&binding)
        {
            mValue = Execution::BindingPtr<T> { std::forward<U>(binding) };
            return *this;
        }

        T get(T defaultValue = {}) const
        {
            return std::visit(overloaded {
                                  [](const T &v) -> T {
                                      return v;
                                  },
                                  [=](const Execution::BindingPtr<T> &b) -> T {
                                      T result = defaultValue;
                                      b.access([&](const T &v) {
                                          result = v;
                                      });
                                      return result;
                                  } },
                mValue);
        }

        operator T() const
        {
            return get();
        }

        friend std::ostream &operator<<(std::ostream &os, const Bindable<T> &b)
        {
            os << b.get();
            return os;
        }

        std::variant<T, BindingPtr<T>> mValue;
    };

    template <>
    struct Bindable<std::string> {

        Bindable() = default;
        
        Bindable(std::string_view s)
            : mValue(std::string {s})
        {
        }

        template <Binding<std::string_view> U>
        Bindable(U &&binding)
            : mValue(Execution::BindingPtr<std::string_view> { std::forward<U>(binding) })
        {
        }

        Bindable &operator=(std::string_view s)
        {
            mValue = std::string { s };
            return *this;
        }

        template <Binding<std::string_view> U>
        Bindable &operator=(U &&binding)
        {
            mValue = Execution::BindingPtr<std::string_view> { std::forward<U>(binding) };
            return *this;
        }

        std::string get(std::string_view defaultValue = "") const
        {
            return std::visit(overloaded {
                                  [](const std::string &s) -> std::string {
                                      return s;
                                  },
                                  [=](const Execution::BindingPtr<std::string_view> &b) -> std::string {
                                      std::string result { defaultValue };
                                      b.access([&](std::string_view sv) {
                                          result = sv;
                                      });
                                      return result;
                                  } },
                mValue);
        }

        operator std::string() const
        {
            return get();
        }

        friend std::ostream &operator<<(std::ostream &os, const Bindable<std::string> &b)
        {
            os << b.get();
            return os;
        }

        std::variant<std::string, BindingPtr<std::string_view>> mValue;
    };

}
}