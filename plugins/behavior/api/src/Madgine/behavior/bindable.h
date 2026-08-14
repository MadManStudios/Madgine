#pragma once

#include "Generic/execution/binding.h"

#include "Meta/serialize/streams/streamresult.h"
#include "Meta/serialize/operations.h"

namespace Engine {
namespace Behavior {

    template <typename T>
    struct Bindable {

        Bindable() = default;

        template <std::convertible_to<T> U>
        Bindable(U &&value)
            : mValue(static_cast<T>(std::forward<U>(value)))
        {
        }

        template <Execution::Binding<T> U>
        Bindable(U &&binding)
            : mValue(Execution::BindingPtr<T> { std::forward<U>(binding) })
        {
        }

        Bindable(std::variant<T, Execution::BindingPtr<T>> value)
            : mValue(std::move(value))
        {
        }

        template <std::convertible_to<T> U>
        Bindable &operator=(U &&value)
        {
            mValue = static_cast<T>(std::forward<U>(value));
            return *this;
        }

        template <Execution::Binding<T> U>
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

        using meta_t = std::variant<T, Execution::BindingPtr<T>>;

        template <bool isReferenceWrapped>
        friend std::variant<T, Execution::BindingPtr<T>> tag_invoke(Reflect::convert_Value_t<isReferenceWrapped>, Bindable<T> &&bindable)
        {
            return std::move(bindable.mValue);
        }

        template <bool isReferenceWrapped>
        friend std::variant<T, Execution::BindingPtr<T>> tag_invoke(Reflect::convert_Value_t<isReferenceWrapped>, Bindable<T> &bindable)
        {
            return bindable.mValue;
        }        

        template <typename Context>
        friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, Bindable &, Serialize::FormattedSerializeStream &, bool, Context &&)
        {
            return {};
        }

        template <typename... Configs, typename Context>
        friend void tag_invoke(Serialize::set_active_t<Configs...>, Bindable &, bool active, bool existenceChanged, Context &&)
        {
        }

        std::variant<T, Execution::BindingPtr<T>> mValue;
    };

    template <>
    struct Bindable<std::string> {

        Bindable() = default;
        
        Bindable(std::string_view s)
            : mValue(std::string {s})
        {
        }

        template <Execution::Binding<std::string_view> U>
        Bindable(U &&binding)
            : mValue(Execution::BindingPtr<std::string_view> { std::forward<U>(binding) })
        {
        }

        Bindable(std::variant<std::string, Execution::BindingPtr<std::string_view>> value)
            : mValue(std::move(value))
        {
        }

        Bindable &operator=(std::string_view s)
        {
            mValue = std::string { s };
            return *this;
        }

        template <Execution::Binding<std::string_view> U>
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

        using meta_t = std::variant<std::string, Execution::BindingPtr<std::string_view>>;

        template <bool isReferenceWrapped>
        friend std::variant<std::string, Execution::BindingPtr<std::string_view>> tag_invoke(Reflect::convert_Value_t<isReferenceWrapped>, Bindable<std::string> &&bindable)
        {
            return std::move(bindable.mValue);
        }

        template <bool isReferenceWrapped>
        friend std::variant<std::string, Execution::BindingPtr<std::string_view>> tag_invoke(Reflect::convert_Value_t<isReferenceWrapped>, Bindable<std::string> &bindable)
        {
            return bindable.mValue;
        }        

        template <typename Context>
        friend Serialize::StreamResult tag_invoke(Serialize::apply_map_t, Bindable &, Serialize::FormattedSerializeStream &, bool, Context &&)
        {
            return {};
        }

        template <typename... Configs, typename Context>
        friend void tag_invoke(Serialize::set_active_t<Configs...>, Bindable &, bool active, bool existenceChanged, Context &&)
        {
        }

        std::variant<std::string, Execution::BindingPtr<std::string_view>> mValue;
    };

}


namespace Serialize {
    template <typename T>
    struct Operations<Behavior::Bindable<T>> {
        template <typename Context>
        static StreamResult read(FormattedSerializeStream &in, Behavior::Bindable<T> &b, const char *name, Context &&context)
        {            
            return Serialize::read(in, b.mValue.template emplace<T>(), name, context);
        }
        template <typename Context>
        static void write(FormattedSerializeStream &out, const Behavior::Bindable<T> &b, const char *name, Context &&context)
        {
            Serialize::write(out, b.get(), name, context);
        }
        static StreamResult visitStream(FormattedSerializeStream &in, const char *name, const StreamVisitor &visitor, size_t depth)
        {
            return Serialize::visitStream<T>(in, name, visitor, depth);
        }
    };
}

}