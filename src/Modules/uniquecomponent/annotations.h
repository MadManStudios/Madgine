#pragma once

#include "Generic/replace.h"

namespace Engine {
namespace Plugins {

    template <typename... Annotations>
    struct GroupedAnnotation : Annotations... {
        template <typename T, typename ActualType>
        GroupedAnnotation(type_holder_t<T> t, type_holder_t<ActualType> at);
    };

    template <typename... Annotations>
    template <typename T, typename ActualType>
    GroupedAnnotation<Annotations...>::GroupedAnnotation(type_holder_t<T> t, type_holder_t<ActualType> at)
        : Annotations(t, at)...
    {
    }

    template <typename R, typename... Args>
    struct ConstructorImpl {
        template <typename T, typename ActualType>
        ConstructorImpl(type_holder_t<T>, type_holder_t<ActualType>)
            : mCtor([](Args &&...args) -> std::unique_ptr<R> {
                return std::make_unique<ActualType>(std::forward<Args>(args)...);
            })
        {
        }

        friend auto tag_invoke(construct_t, const ConstructorImpl &object, Args &&...args)
        {
            return object.mCtor(std::forward<Args>(args)...);
        }

        std::unique_ptr<R> (*mCtor)(Args &&...);
    };

    template <typename... Args>
    using Constructor = ConstructorImpl<Placeholder<0>, Args...>;

    template <typename R>
    struct CopyConstructorImpl {
        template <typename T, typename ActualType>
        CopyConstructorImpl(type_holder_t<T>, type_holder_t<ActualType>)
            : mCtor([](const R &other) -> std::unique_ptr<R> {
                return std::make_unique<ActualType>(static_cast<const ActualType &>(other));
            })
        {
        }

        friend auto tag_invoke(construct_t, const CopyConstructorImpl &object, const R &other)
        {
            return object.mCtor(other);
        }

        std::unique_ptr<R> (*mCtor)(const R&);
    };
    
    using CopyConstructor = CopyConstructorImpl<Placeholder<0>>;

    template <typename Base = Placeholder<0>>
    struct Destructor {

        template <typename T, typename ActualType>
        Destructor(type_holder_t<T>, type_holder_t<ActualType>)
            : mDtor([](Base *object) {
                delete static_cast<ActualType *>(object);
            })
        {
        }

        void destroy(Base *objectToDestroy) const
        {
            return mDtor(objectToDestroy);
        }

        void (*mDtor)(Base *);
    };

    template <typename Base = Placeholder<0>>
    struct Copying {

        template <typename T, typename ActualType>
        Copying(type_holder_t<T>, type_holder_t<ActualType>)
            : mCopy([](Base &target, const Base &source) {
                static_cast<ActualType &>(target) = static_cast<const ActualType &>(source);
            })
        {
        }

        void copy(Base &target, const Base &source) const
        {
            return mCopy(target, source);
        }

        void (*mCopy)(Base &, const Base &);
    };

    template <typename R, typename... Args>
    struct FactoryImpl {
        template <typename T, typename ActualType>
        FactoryImpl(type_holder_t<T>, type_holder_t<ActualType>)
            : mFactory(T::factory)
        {
        }

        R create(Args &&...args) const
        {
            return mFactory(std::forward<Args>(args)...);
        }

        R(*mFactory)
        (Args &&...);
    };

    template <typename... Args>
    using Factory = FactoryImpl<Placeholder<0>, Args...>;
}
}