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
#include "boundapifunction.h"
#include "keyvaluebinding.h"
#include "keyvaluefunction.h"
#include "keyvaluesender.h"
#include "keyvaluevirtualrange.h"
#include "objectptr.h"
#include "ownedscopeptr.h"
#include "scopeptr.h"
#include "valuetype_desc.h"
#include "valuetype_forward.h"

namespace Engine {

DERIVE_OPERATOR(Equal, ==);

struct META_EXPORT ValueType {

    using Union = std::variant<
#define VALUETYPE_SEP ,
#define VALUETYPE_TYPE(Name, Storage, ...) Storage
#include "valuetypedefinclude.h"
        >;

    ValueType();

    ValueType(const ValueType &other);

    ValueType(ValueType &&other) noexcept;

    template <DecayedNoneOf<ValueType> T>
    explicit ValueType(T &&v)
        : mUnion(std::in_place_index<static_cast<size_t>(static_cast<ValueTypeEnum>(toValueTypeIndex<std::decay_t<T>>()))>, std::forward<T>(v))
    {
    }

    ~ValueType();

    void clear();

    void operator=(const ValueType &other);
    void operator=(ValueType &&other);

    template <DecayedNoneOf<ValueType> T>
    void operator=(T &&t)
    {
        static_assert(!requires { typename std::decay_t<T>::no_value_type; });

        constexpr size_t index = static_cast<size_t>(static_cast<ValueTypeEnum>(toValueTypeIndex<std::decay_t<T>>()));
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

    ValueTypeIndex index() const;
    ValueTypeDesc type() const;

    void setType(ValueTypeDesc type);

    KeyValueResult call(ValueType &retVal, const ArgumentList &args) const;
    template <typename... Args>
    KeyValueResult call(ValueType &retVal, Args &&...args)
    {
        return call(retVal, { ValueType { std::forward<Args>(args) }... });
    }

private:
    Union mUnion;
};

template <typename T>
bool ValueType::is() const
{
    return toValueTypeDesc<T>().canAccept(type());
}

template <typename T>
const T &ValueType::as() const
{
    return std::get<T>(mUnion);
}

template <typename T>
T &ValueType::as()
{
    return std::get<T>(mUnion);
}

META_EXPORT std::ostream &operator<<(std::ostream &stream,
    const Engine::ValueType &v);

}
