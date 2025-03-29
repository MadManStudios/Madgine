#pragma once

#include "sortedcontainerapi.h"
#include "underlying_container.h"

namespace Engine {

template <typename C, typename Base>
struct container_api_impl : container_api_impl<C, typename underlying_container<Base>::type> {
    using container_api_impl<C, typename underlying_container<Base>::type>::operator=;
};

template <typename C>
using container_api = container_api_impl<C, C>;

template <typename C, typename Base>
struct underlying_container<container_api_impl<C, Base>> {
    typedef C type;
};

template <typename C, typename... Ty>
struct container_api_impl<C, std::list<Ty...>> : C {

    using C::C;

    using C::operator=;

    void remove(const typename C::value_type &item)
    {
        for (typename C::iterator it = this->begin(); it != this->end();) {
            if (*it == item) {
                it = this->erase(it);
            } else {
                ++it;
            }
        }
    }

    decltype(auto) push_back(const typename C::value_type &item)
    {
        return emplace_back(item);
    }

    template <typename... _Ty>
    decltype(auto) emplace_back(_Ty &&...args)
    {
        return emplace(this->end(), std::forward<_Ty>(args)...);
    }

    template <typename... _Ty>
    decltype(auto) emplace(const typename C::iterator &it, _Ty &&...args)
    {
        return Engine::emplace(*this, it, std::forward<_Ty>(args)...);
    }

    const typename C::value_type &back() const
    {
        return C::back();
    }

    typename C::value_type &back()
    {
        return C::back();
    }
};

template <typename C, typename... Ty>
struct container_api_impl<C, std::vector<Ty...>> : C {

    using C::C;

    using C::operator=;

    using value_type = typename C::value_type;

    /*void resize(size_t size)
            {
                C::resize(size);
            }*/

    void remove(const value_type &item)
    {
        for (typename C::const_iterator it = this->begin(); it != this->end();) {
            if (*it == item) {
                it = erase(it);
            } else {
                ++it;
            }
        }
    }

    decltype(auto) push_back(const value_type &item)
    {
        return *emplace_back(item);
    }

    decltype(auto) push_back(value_type &&item)
    {
        return *emplace_back(std::move(item));
    }

    template <typename... _Ty>
    decltype(auto) emplace_back(_Ty &&...args)
    {
        bool success;
        return Engine::emplace(success, *this, this->end(), std::forward<_Ty>(args)...);
    }

    value_type &at(size_t i)
    {
        return C::at(i);
    }

    const value_type &at(size_t i) const
    {
        return C::at(i);
    }

    value_type &operator[](size_t i)
    {
        return C::operator[](i);
    }

    const value_type &operator[](size_t i) const
    {
        return C::operator[](i);
    }
};

template <typename C, typename... Ty>
struct container_api_impl<C, std::deque<Ty...>> : C {

    using C::C;

    using C::operator=;

    using value_type = typename C::value_type;

    /*void resize(size_t size)
            {
                C::resize(size);
            }*/

    void remove(const value_type &item)
    {
        for (typename C::const_iterator it = this->begin(); it != this->end();) {
            if (*it == item) {
                it = erase(it);
            } else {
                ++it;
            }
        }
    }

    decltype(auto) push_back(const value_type &item)
    {
        return *emplace_back(item);
    }

    decltype(auto) push_back(value_type &&item)
    {
        return *emplace_back(std::move(item));
    }

    template <typename... _Ty>
    decltype(auto) emplace_back(_Ty &&...args)
    {
        return *emplace(static_cast<C &>(*this), this->end(), std::forward<_Ty>(args)...);
    }

    value_type &at(size_t i)
    {
        return C::at(i);
    }

    const value_type &at(size_t i) const
    {
        return C::at(i);
    }

    value_type &operator[](size_t i)
    {
        return C::operator[](i);
    }

    const value_type &operator[](size_t i) const
    {
        return C::operator[](i);
    }
};

template <typename C, typename... Ty>
struct container_api_impl<C, std::set<Ty...>> : SortedContainerApi<C> {
    using C::operator=;
};

template <typename C, typename K, typename T, typename... Ty>
struct container_api_impl<C, std::map<K, T, Ty...>> : SortedContainerApi<C> {
    using SortedContainerApi<C>::SortedContainerApi;

    using C::operator=;

    T &operator[](const K &key)
    {
        typename C::iterator it = C::lower_bound(key);
        if (it == this->end() || it->first != key) {
            auto pib = try_emplace(key);
            assert(pib.second);
            it = pib.first;
        }
        return it->second;
    }

    template <typename... _Ty>
    auto try_emplace(const K &key, _Ty &&...args) -> decltype(emplace(std::declval<C &>(), C::lower_bound(key), std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<_Ty>(args)...)))
    {
        auto where = C::lower_bound(key);
        bool success;
        auto it = emplace(success, static_cast<C &>(*this), where, std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<_Ty>(args)...));
        return std::make_pair(it, success);
    }
};

}