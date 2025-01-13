#pragma once

#include "serializableunit.h"

#include "serializableunitptr.h"

namespace Engine {
namespace Serialize {

    template <typename _Base = SerializableUnitBase, typename... _Bases>
    struct DLL_EXPORT VirtualSerializableUnitBase : _Base, _Bases... {

        using _Base::_Base;

        virtual SerializableUnitPtr customUnitPtr() = 0;
        virtual SerializableUnitConstPtr customUnitPtr() const = 0;
    };

    template <typename T, typename _Base = VirtualSerializableUnitBase<>>
    struct VirtualUnit : _Base {
        using _Base::_Base;
        virtual ~VirtualUnit() = default;
        virtual SerializableUnitPtr customUnitPtr() override
        {
            return { this, &serializeTable<decayed_t<T>>() };
        }
        virtual SerializableUnitConstPtr customUnitPtr() const override
        {
            return { this, &serializeTable<decayed_t<T>>() };
        }
    };

    template <typename _Base = void>
    struct DLL_EXPORT VirtualSerializableDataBase : _Base {

        using _Base::_Base;

        virtual SerializableDataPtr customUnitPtr() = 0;
        virtual SerializableDataConstPtr customUnitPtr() const = 0;
    };

    template <typename T, typename _Base = VirtualSerializableDataBase<>>
    struct VirtualData : _Base {
        using _Base::_Base;
        virtual ~VirtualData() = default;
        virtual SerializableDataPtr customUnitPtr() override
        {
            return { static_cast<decayed_t<T>*>(this), &serializeTable<decayed_t<T>>() };
        }
        virtual SerializableDataConstPtr customUnitPtr() const override
        {
            return { static_cast<const decayed_t<T> *>(this), &serializeTable<decayed_t<T>>() };
        }
    };

} // namespace Serialize
} // namespace Core
