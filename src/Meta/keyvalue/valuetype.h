#pragma once

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
#include "keyvaluefunction.h"

#include "keyvaluevirtualrange.h"

#include "objectptr.h"

#include "typedscopeptr.h"

#include "ownedscopeptr.h"

#include "Generic/cow.h"
#include "Generic/cowstring.h"

#include "keyvaluesender.h"

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
    ValueType_Return<T> as() const;

    template <typename T>
    ValueType_Return<T> asDefault(const T &defaultValue)
    {
        if (!is<T>()) {
            mUnion = defaultValue;
        }
        return as<T>();
    }

    bool isReference() const;

    ValueTypeIndex index() const;
    ValueTypeDesc type() const;

    void setType(ValueTypeDesc type);

    void call(ValueType &retVal, const ArgumentList &args) const;
    template <typename... Args>
    void call(ValueType &retVal, Args &&...args)
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
ValueType_Return<T> ValueType::as() const
{

    if constexpr (ValueTypePrimitive<T>) {
        return visit([](const auto &v) -> ValueType_Return<T> {
            if constexpr (std::is_convertible_v<decltype(v), ValueType_Return<T>>)
                return v;
            else
                throw 0;
        });
        //return std::get<static_cast<size_t>(toValueTypeIndex<T>().mIndex)>(mUnion);
    } else if constexpr (std::same_as<T, ValueType>) {
        return *this;
    } else if constexpr (Enum<T>) {
        return static_cast<T>(std::get<std::underlying_type_t<T>>(mUnion));
    } else {
        if constexpr (Pointer<T>) {
            return scope_cast<std::remove_pointer_t<T>>(std::get<ScopePtr>(mUnion));
        } else {
            return scope_cast<std::remove_reference_t<T>>(std::get<OwnedScopePtr>(mUnion));
        }
    }
    //static_assert(dependent_bool<T, false>::value, "Invalid target type for Valuetype cast provided!");
}

META_EXPORT std::ostream &operator<<(std::ostream &stream,
    const Engine::ValueType &v);

}
