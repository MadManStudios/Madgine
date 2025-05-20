#pragma once

#include "Generic/replace.h"

namespace Engine {
namespace UniqueComponent {

    template <typename... Annotations>
    struct GroupedAnnotation : Annotations...{
        template <typename T, typename ActualType>
        GroupedAnnotation(type_holder_t<T> t, type_holder_t<ActualType> at)
            : Annotations(t, at)...
        {
        }
    };

    template <typename R, typename... Args>
	struct ConstructorImpl {
        template <typename T, typename ActualType>
        ConstructorImpl(type_holder_t<T>, type_holder_t<ActualType>)
            : mCtor([](Args &&...args) -> R{
                return std::make_unique<ActualType>(std::forward<Args>(args)...);
            })
        {
        }

        friend auto tag_invoke(construct_t, const ConstructorImpl &object, Args &&...args)
        {
            return object.mCtor(std::forward<Args>(args)...);
        }

        R (*mCtor)(Args&&...);
    };

    template <typename... Args>
    using Constructor = ConstructorImpl<Placeholder<0>, Args...>;


    template <typename R, typename... Args>
	struct FactoryImpl {
        template <typename T, typename ActualType>
        FactoryImpl(type_holder_t<T>, type_holder_t<ActualType>)
            : mFactory(T::factory)
        {
        }

        R create(Args&&... args) const
        {
            return mFactory(std::forward<Args>(args)...);
        }

        R (*mFactory)(Args&&...);
    };

    template <typename... Args>
    using Factory = FactoryImpl<Placeholder<0>, Args...>;

}
}