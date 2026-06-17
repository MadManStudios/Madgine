#pragma once

#include "Meta/reflect/util.h"

namespace Engine {
namespace Execution {
    namespace State {

        struct MADGINE_DEBUGGER_EXPORT Value {

            template <typename V>
            Value(std::string_view name, V &&v)
                : mName(name)
                , mPtr(&v)
                , mGet([](Reflect::Value &out, void *v) {
                    Reflect::toValue(out, *static_cast<const std::remove_reference_t<V> *>(v));
                })
                , mSet([](void *v, const Reflect::Value &in) {
                    Reflect::invoke([v](const std::remove_reference_t<V> &value) { *static_cast<std::remove_reference_t<V> *>(v) = value; }, in);
                })
                , mType(Reflect::toType<std::remove_reference_t<V>>())
            {
            }

            void get(Reflect::Value &out) const;
            void set(const Reflect::Value &in) const;

            std::string_view mName;
            void *mPtr;
            void (*mGet)(Reflect::Value &, void *);
            void (*mSet)(void *, const Reflect::Value &);
            Reflect::ExtendedType mType;
        };

    }
}
}