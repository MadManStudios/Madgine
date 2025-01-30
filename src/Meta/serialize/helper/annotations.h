#pragma once

namespace Engine {
namespace Serialize {

struct TypeAnnotation {

	template <typename T, typename ActualType>
    TypeAnnotation(type_holder_t<T>, type_holder_t<ActualType>)
        : mType(&serializeTable<T>())
    {
    }

	const SerializeTable *mType;
};

}
}