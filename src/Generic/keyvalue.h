#pragma once

#include "comparator_traits.h"

namespace Engine {

template <typename T>
struct KeyValue {
    static T &value(T &v)
    {
        return v;
    }

    static T value(T &&v)
    {
        return std::move(v);
    }

    static Void key(T &v)
    {
        static_assert(sizeof(std::remove_reference_t<T>) > 0, "Cannot determine key of incomplete type!");
        return {};
    }

    static Void key(T &&v)
    {
        static_assert(sizeof(std::remove_reference_t<T>) > 0, "Cannot determine key of incomplete type!");
        return {};
    }
};

template <typename T>
concept HasKey = requires(T &t)
{
    t.key();
};

template <HasKey T>
struct KeyValue<T> {
    static T &value(T &v)
    {
        return v;
    }

    static decltype(auto) key(const T &v)
    {
        return v.key();
    }
};

template <HasKey T>
struct KeyValue<T *> {
    static T *value(T *v)
    {
        return v;
    }

    static decltype(auto) key(const T *v)
    {
        return v->key();
    }
};

template <HasKey T>
struct KeyValue<T *const> {
    static T *value(T *v)
    {
        return v;
    }

    static decltype(auto) key(const T *v)
    {
        return v->key();
    }
};

template <typename K, typename T>
struct KeyValue<std::pair<K, T>> {

    static T value(std::pair<K, T>&& p) {
        return std::move(p.second);
    }

    static T &value(std::pair<K, T> &p)
    {
        return p.second;
    }

    static const K &key(const std::pair<K, T> &p)
    {
        return p.first;
    }
};

template <typename K, typename T>
struct KeyValue<const std::pair<K, T>> {
    static const T &value(const std::pair<K, T> &p)
    {
        return p.second;
    }

    static const K &key(const std::pair<K, T> &p)
    {
        return p.first;
    }
};

template <typename T, typename D>
struct KeyValue<std::unique_ptr<T, D>> {
    static T *value(const std::unique_ptr<T, D> &p) // TODO
    {
        return p.get();
    }

    static decltype(auto) key(const std::unique_ptr<T, D> &p)
    {
        static_assert(sizeof(std::remove_reference_t<T>) > 0, "Cannot determine key of incomplete type!");
        return KeyValue<T>::key(*p);
    }
};

template <typename T, typename D>
struct KeyValue<const std::unique_ptr<T, D>> {
    static T *value(const std::unique_ptr<T, D> &p) // TODO
    {
        return p.get();
    }

    static decltype(auto) key(const std::unique_ptr<T, D> &p)
    {
        static_assert(sizeof(std::remove_reference_t<T>) > 0, "Cannot determine key of incomplete type!");
        return KeyValue<T>::key(*p);
    }
};

template <typename T>
struct KeyValue<std::reference_wrapper<T>> {
    using Inner = KeyValue<T>;

    static decltype(auto) value(std::reference_wrapper<T> ref) {
        if constexpr (std::is_reference_v<decltype(Inner::value(ref.get()))>) {
            return std::ref(Inner::value(ref.get()));
        }else{
            return Inner::value(ref.get());            
        }
    }

    static decltype(auto) key(std::reference_wrapper<T> ref)
    {
        if constexpr (std::is_reference_v<decltype(Inner::key(ref.get()))>) {
            return std::ref(Inner::key(ref.get()));
        } else {
            return Inner::key(ref.get());
        }
    }
};

template <typename T>
decltype(auto) kvValue(T &&v)
{
    return KeyValue<std::remove_reference_t<T>>::value(std::forward<T>(v));
}

constexpr auto projectionValue = LIFT(kvValue);

template <typename T>
decltype(auto) kvKey(T &&v)
{
    return KeyValue<std::remove_reference_t<T>>::key(std::forward<T>(v));
}

constexpr auto projectionKey = LIFT(kvKey);

template <typename T>
auto kvValues(T &v)
{
    return std::views::transform(v, projectionValue);
}

template <typename T>
auto kvKeys(T &v)
{
    return std::views::transform(v, projectionKey);
}

template <typename T>
struct KeyType {
    typedef std::decay_t<decltype(kvKey(std::declval<T>()))> type;
};

template <typename T>
using KeyType_t = typename KeyType<T>::type;

template <typename T>
using ValueType_t = std::decay_t<decltype(kvValue(std::declval<T>()))>;

template <typename _Ty>
struct KeyCompare {

    using is_transparent = void;

    template <typename T1, typename T2>
    constexpr bool operator()(const T1 &_Left, const T2 &_Right) const
    {
        if constexpr (std::same_as<T1, KeyType_t<_Ty>> && std::same_as<T2, KeyType_t<_Ty>>)
            return _Left < _Right;
        else if constexpr (std::same_as<T1, KeyType_t<_Ty>>)
            return _Left < kvKey(_Right);
        else if constexpr (std::same_as<T2, KeyType_t<_Ty>>)
            return kvKey(_Left) < _Right;
        else
            return kvKey(_Left) < kvKey(_Right);
    }

};

template <typename _Ty>
struct comparator_traits<KeyCompare<_Ty>> {
    typedef KeyType_t<_Ty> type;
    typedef _Ty item_type;

    static decltype(auto) to_cmp_type(const _Ty &v)
    {
        return kvKey(v);
    }
};

template <typename _Ty = void>
struct KeyHash {
    typedef _Ty argument_type;
    typedef size_t result_type;

    constexpr size_t operator()(const _Ty &_Arg) const
    {
        return std::hash<KeyType_t<_Ty>> {}(kvKey(_Arg));
    }
};

}
