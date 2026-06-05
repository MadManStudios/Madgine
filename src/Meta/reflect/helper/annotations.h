#pragma once

#include "../table_forward.h"

namespace Engine {
namespace Reflect {

    struct TypeAnnotation {

        template <typename T, typename ActualType>
        TypeAnnotation(type_holder_t<T>, type_holder_t<ActualType>)
            : mType(&table<T>)
        {
        }

        const Reflect::MetaTable **mType;
    };

}
}