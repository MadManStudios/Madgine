#pragma once

#include "uniquecomponentregistry.h"

#include "uniquecomponent.h"

#include "Generic/container/emplace.h"

namespace Engine {
namespace UniqueComponent {

    template <typename C, typename Registry>
    struct Container : C {

        typedef typename Registry::Base Base;

        template <typename... Args>
        requires requires(typename Registry::Annotations &a, Args&&... args) {
            a.construct(args...);
        }
        Container(Args&&... arg)
        {
            size_t count = Registry::sComponents().size();
            mSortedComponents.reserve(count);
            if constexpr (InstanceOf<C, std::vector>) {
                this->reserve(count);
            }
            for (const auto &annotations : Registry::sComponents()) {
                auto p = annotations.construct(arg...);
                mSortedComponents.push_back(p.get());
                Engine::emplace(static_cast<C &>(*this), C::end(), std::move(p));
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

template <typename C, typename Registry>
struct underlying_container<UniqueComponent::Container<C, Registry>> {
    typedef C type;
};

}