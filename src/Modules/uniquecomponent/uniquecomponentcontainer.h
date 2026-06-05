#pragma once

#include "Generic/containers/emplace.h"

#include "uniquecomponent.h"
#include "uniquecomponentregistry.h"

namespace Engine {
namespace Plugins {

    template <typename C, typename Registry, typename Base>
    struct Container : C {

        template <typename... Args>
            requires tag_invocable<construct_t, const typename Registry::Annotations, Args...>
        Container(Args &&...arg)
        {
            size_t count = Registry::sComponents().size();
            mSortedComponents.reserve(count);
            if constexpr (Concepts::InstanceOf<C, std::vector>) {
                this->reserve(count);
            }
            for (const auto &annotations : Registry::sComponents()) {
                auto p = construct(annotations, arg...);
                mSortedComponents.push_back(p.get());
                Containers::emplace(static_cast<C &>(*this), C::end(), std::move(p));
            }
        }

        Container(const Container &) = delete;
        void operator=(const Container &) = delete;

        template <typename T>
        T &get()
        {
            return static_cast<T &>(get(component_index<T>()));
        }

        Base &get(size_t i)
        {
            return *mSortedComponents[i];
        }

    private:
        std::vector<Base *> mSortedComponents;
    };
}

template <typename C, typename Registry, typename Base>
struct Containers::underlying_container<Plugins::Container<C, Registry, Base>> {
    typedef C type;
};

}