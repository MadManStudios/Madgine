#pragma once

#include "uniquecomponentregistry.h"

namespace Engine {
namespace UniqueComponent {

    template <typename Registry>
    struct Selector {
        typedef typename Registry::Base Base;

        template <typename... Args>
        Selector(Args&&... args)
            : mValue(Registry::sComponents().at(0).construct(std::forward<Args>(args)...))
        {
        }

        Selector(const Selector &) = delete;
        void operator=(const Selector &) = delete;

        Base *operator->() const
        {
            return mValue.get();
        }

        operator Base *() const
        {
            return mValue.get();
        }

        Base *get() const
        {
            return mValue.get();
        }

    private:
        std::unique_ptr<Base> mValue;
    };

}
}