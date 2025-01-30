#pragma once

#include "component_index.h"

#if ENABLE_PLUGINS

#    include "indexholder.h"

namespace Engine {
namespace UniqueComponent {

    DLL_IMPORT_VARIABLE2(Engine::UniqueComponent::IndexHolder, _reg, typename T);
    DLL_IMPORT_VARIABLE2(Engine::UniqueComponent::IndexHolder *, _preg, typename T);

    template <typename _T, typename _Base, typename _VBase>
    struct VirtualComponentImpl : _Base {
        using VBase = _VBase;
        using T = _T;
        using _Base::_Base;

        template <typename ActualType>
        struct Registrator : ActualType::Collector::template ComponentRegistrator<T, ActualType> {
            Registrator()
            {
                if (!_Base::preg())
                    _Base::preg() = this;
            }
            ~Registrator()
            {
                if (_Base::preg() == this)
                    _Base::preg() = nullptr;
            }
        };

        static size_t component_index()
        {
            return _reg<T>().index();
        }
    };

    template <typename _T, typename _Collector, typename _Base>
    struct VirtualComponentBase : public _Base {
    public:
        using Collector = _Collector;
        using T = _T;

        using _Base::_Base;

        static size_t component_index()
        {
            return preg()->index();
        }

        static bool is_instantiated()
        {
            return preg();
        }

    protected:
        static IndexHolder *&preg()
        {
            return _preg<T>();
        }
    };

    template <typename _T, typename _Collector, typename _Base>
    struct Component : _Base {
        using Collector = _Collector;
        using T = _T;

        using _Base::_Base;

        static size_t component_index()
        {
            return _reg<T>().index();
        }
        
        template <typename ActualType>
        using Registrator = typename ActualType::Collector::template ComponentRegistrator<T, ActualType>;
    };

#    define UNIQUECOMPONENT(Type) DLL_EXPORT_VARIABLE3(, Engine::UniqueComponent::IndexHolder, Type::Registrator<Type>, Engine::UniqueComponent::, _reg, , {}, Type::T)
#    define UNIQUECOMPONENT2(Type, ext) DLL_EXPORT_VARIABLE3(, Engine::UniqueComponent::IndexHolder, Type::Registrator<Type>, Engine::UniqueComponent::, _reg, ext, {}, Type::T)

#    define VIRTUALUNIQUECOMPONENTBASE(Name) DLL_EXPORT_VARIABLE2(, Engine::UniqueComponent::IndexHolder *, Engine::UniqueComponent::, _preg, nullptr, Name)


}
}

#else

namespace Engine {
namespace UniqueComponent {
    template <typename _T, typename _Base, typename _VBase>
    struct VirtualComponentImpl : _Base {
        using _Base::_Base;

        using T = _T;
    };

    template <typename _T, typename Collector, typename _Base>
    struct VirtualComponentBase : _Base {
        using _Base::_Base;

        static bool is_instantiated() {
            return true;
        }

        using T = _T;
    };

    template <typename _T, typename Collector, typename _Base>
    struct Component : _Base {
        using _Base::_Base;

        using T = _T;
    };

}
}

#    define UNIQUECOMPONENT(Type)
#    define UNIQUECOMPONENT2(Name, ext)
#    define VIRTUALUNIQUECOMPONENTBASE(Name)

#endif

namespace Engine {
namespace UniqueComponent {

    DLL_IMPORT_VARIABLE2(const std::string_view, _componentName, typename T);

    template <typename _Base>
    struct NamedComponent : _Base {
        using _Base::_Base;

        static std::string_view componentName()
        {
            return _componentName<typename _Base::T>();
        }
    };

}
}

#define COMPONENT_NAME(Name, FrontendType) \
    DLL_EXPORT_VARIABLE2(constexpr, const std::string_view, Engine::UniqueComponent::, _componentName, #Name, FrontendType);

#define NAMED_UNIQUECOMPONENT(Name, Type) \
    COMPONENT_NAME(Name, Type)            \
    UNIQUECOMPONENT(Type)
