#pragma once

#include "../comparator_traits.h"

#include "emplace.h"

namespace Engine {

template <typename C>
struct SortedContainerApi : C {

    using C::C;
    using C::operator=;

    /*iterator find(const key_type& key)
			{
				return kvFind(this->mData, key);
			}*/

    template <typename T>
    bool contains(const T &key)
    {
        return this->find(key) != this->end();
    }

    template <typename... _Ty>
    auto emplace(_Ty &&... args)
    {
        return Engine::emplace(static_cast<C&>(*this), this->end(), std::forward<_Ty>(args)...);
    }

    /*template <class Ty, class _ = decltype(std::declval<typename C::NativeContainerType>().find(std::declval<Ty>()))>
			iterator find(const Ty& v)
			{
				return this->mData.find(v);
			}*/
};

template <typename C>
struct underlying_container<SortedContainerApi<C>> {
    using type = C;
};
}
