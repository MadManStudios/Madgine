#pragma once

#include "../any.h"
#include "atomiccontaineroperation.h"

namespace Engine {
namespace Containers {

    struct CompoundAtomicOperation {

        ~CompoundAtomicOperation()
        {
            for (Any &holder : mHolder) {
                holder = Any {};
            }
        }

        template <typename C, typename... Args>
        decltype(auto) addInsertOperation(C &c, Args &&...args)
        {
            if constexpr (has_typename_InsertOperation<C>)
                return mHolder.emplace_back(Any::inplace<typename C::InsertOperation>, c, std::forward<Args>(args)...).template as<typename C::InsertOperation>();
            else
                return c;
        }

        template <typename C, typename... Args>
        decltype(auto) addMultiInsertOperation(C &c, Args &&...args)
        {
            if constexpr (has_typename_MultiInsertOperation<C>)
                return mHolder.emplace_back(Any::inplace<typename C::MultiInsertOperation>, c, std::forward<Args>(args)...).template as<typename C::MultiInsertOperation>();
            else
                return c;
        }

        template <typename C, typename... Args>
        decltype(auto) addRemoveOperation(C &c, Args &&...args)
        {
            if constexpr (has_typename_RemoveOperation<C>)
                return mHolder.emplace_back(Any::inplace<typename C::RemoveOperation>, c, std::forward<Args>(args)...).template as<typename C::RemoveOperation>();
            else
                return c;
        }

    private:
        std::vector<Any> mHolder;
    };

}
}