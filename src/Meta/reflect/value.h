#pragma once

#include "Generic/cow.h"
#include "Generic/cowstring.h"

#include "../math/color3.h"
#include "../math/color4.h"
#include "../math/matrix3.h"
#include "../math/matrix4.h"
#include "../math/quaternion.h"
#include "../math/vector2.h"
#include "../math/vector2i.h"
#include "../math/vector3.h"
#include "../math/vector3i.h"
#include "../math/vector4.h"
#include "../math/vector4i.h"
#include "binding.h"
#include "boundapifunction.h"
#include "function.h"
#include "objectptr.h"
#include "ownedvalue.h"
#include "range.h"
#include "scopeptr.h"
#include "sender.h"
#include "type.h"
#include "util.h"

namespace Engine {
namespace Reflect {

    DERIVE_OPERATOR(Equal, ==);

    struct META_EXPORT Value {

        using Union = std::variant<
#define VALUE_SEP ,
#define VALUE_TYPE(Name, Storage, ...) Storage
#include "valuedefinclude.h"
            >;

        Value();

        Value(const Value &other);

        Value(Value &&other) noexcept;

        template <Concepts::DecayedNoneOf<Value> T>
        explicit Value(T &&v)
            : mUnion(std::in_place_index<static_cast<size_t>(static_cast<TypeEnum>(toTypeIndex<std::decay_t<T>>()))>, std::forward<T>(v))
        {
        }

        ~Value();

        void clear();

        void operator=(const Value &other);
        void operator=(Value &&other);

        template <Concepts::DecayedNoneOf<Value> T>
        void operator=(T &&t)
        {
            static_assert(!requires { typename std::decay_t<T>::no_value_type; });

            constexpr size_t index = static_cast<size_t>(static_cast<TypeEnum>(toTypeIndex<std::decay_t<T>>()));
            if (mUnion.index() == index)
                std::get<index>(mUnion) = std::forward<T>(t);
            else
                mUnion.emplace<index>(std::forward<T>(t));
        }

        template <typename V>
        bool operator==(const V &v) const
        {
            return visit([&]<typename U>(const U &u) {
                if constexpr (has_operator_Equal<V, U>)
                    return v == u;
                else
                    return false;
            });
        }

        template <typename T>
        bool operator!=(const T &other) const
        {
            return !(*this == other);
        }

        std::string toShortString() const;

        std::string getTypeString() const;

        template <typename V>
        decltype(auto) visit(V &&visitor) const &
        {
            return std::visit(std::forward<V>(visitor), mUnion);
        }

        template <typename V>
        decltype(auto) visit(V &&visitor) &
        {
            return std::visit(std::forward<V>(visitor), mUnion);
        }

        template <typename V>
        decltype(auto) visit(V &&visitor) const &&
        {
            return std::visit(std::forward<V>(visitor), std::move(mUnion));
        }

        template <typename V>
        decltype(auto) visit(V &&visitor) &&
        {
            return std::visit(std::forward<V>(visitor), std::move(mUnion));
        }

        template <typename T>
        bool is() const;

        template <typename T>
        const T &as() const;

        template <typename T>
        T &as();

        template <typename T>
        T &asDefault(const T &defaultValue)
        {
            if (!is<T>()) {
                mUnion = defaultValue;
            }
            return as<T>();
        }

        TypeIndex index() const;
        Type type() const;

        void setType(Type type);

        Result call(Value &retVal, const ArgumentList &args) const;
        template <Concepts::DecayedNoneOf<ArgumentList>... Args>
        Result call(Value &retVal, Args &&...args)
        {
            return call(retVal, { Value { std::forward<Args>(args) }... });
        }

        ScopeIterator end() const;

    private:
        Union mUnion;
    };

    template <typename T>
    bool Value::is() const
    {
        return toType<T>().canAccept(type());
    }

    template <typename T>
    const T &Value::as() const
    {
        return std::get<T>(mUnion);
    }

    template <typename T>
    T &Value::as()
    {
        return std::get<T>(mUnion);
    }

    META_EXPORT std::ostream &operator<<(std::ostream &stream,
        const Value &v);

}
}