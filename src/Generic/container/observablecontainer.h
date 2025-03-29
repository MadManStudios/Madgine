#pragma once

#include "../container/containerevent.h"

namespace Engine {

template <typename C, typename Observer>
struct ObservableContainer : C, Observer {

    using _traits = container_traits<C>;
    
    typedef typename _traits::iterator iterator;
    typedef typename _traits::const_iterator const_iterator;

    template <typename... _Ty>
    typename _traits::emplace_return emplace(const iterator &where, _Ty &&... args)
    {
        Observer::operator()(where, BEFORE | EMPLACE);
        typename _traits::emplace_return it = _traits::emplace(*this, where, std::forward<_Ty>(args)...);
        Observer::operator()(it, (_traits::was_emplace_successful(it) ? AFTER : ABORTED) | EMPLACE);
        return it;
    }

    iterator erase(const iterator &where)
    {
        Observer::operator()(where, BEFORE | ERASE);
        iterator it = _traits::container::erase(where);
        Observer::operator()(it, AFTER | ERASE);
        return it;
    }

    iterator erase(const iterator &from, const iterator &to)
    {
        Observer::operator()(from, BEFORE | ERASE_RANGE);
        size_t count = 0;
        for (iterator it = from; it != to; ++it) {
            Observer::operator()(it, BEFORE | ERASE);
            ++count;
        }
        iterator it = _traits::container::erase(from, to);
        for (; count > 0; --count) {
            Observer::operator()(it, AFTER | ERASE);
        }
        Observer::operator()(it, AFTER | ERASE_RANGE);
        return it;
    }

    void clear()
    {
        Observer::operator()(this->begin(), BEFORE | RESET);
        _traits::container::clear();
        Observer::operator()(this->end(), AFTER | RESET);
    }

    template <typename T>
    ObservableContainer<C, Observer> &operator=(T &&t)
    {
        Observer::operator()(this->begin(), BEFORE | RESET);
        _traits::container::operator=(std::forward<T>(t));
        Observer::operator()(this->end(), AFTER | RESET);
        return *this;
    }
};

}