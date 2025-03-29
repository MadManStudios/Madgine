#pragma once

namespace Engine {

using ValueTypeList = type_pack<
#define VALUETYPE_SEP ,
#define VALUETYPE_TYPE(Name, Storage, ...) type_pack<__VA_ARGS__>::transform<std::decay_t>
#include "valuetypedefinclude.h"
    >;

using QualifiedValueTypeList = type_pack<
#define VALUETYPE_SEP ,
#define VALUETYPE_TYPE(Name, Storage, ...) type_pack<__VA_ARGS__>
#include "valuetypedefinclude.h"
    >;

}