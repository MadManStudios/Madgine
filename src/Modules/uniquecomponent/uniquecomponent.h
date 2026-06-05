#pragma once

#include "component_index.h"

#if ENABLE_PLUGINS

#    include "indexholder.h"

namespace Engine {
namespace Plugins {

    DLL_IMPORT_VARIABLE2(IndexHolder, _reg, typename T);
    DLL_IMPORT_VARIABLE2(IndexHolder *, _preg, typename T);

    template <typename _T, typename _Base, typename _VBase>
    struct VirtualComponentImpl : _Base {
        using VBase = _VBase;
        using T = _T;
        using _Base::_Base;

        template <typename ActualType>
        struct Registrator : ActualType::Collector::template ComponentRegistrator<T, ActualType> {
            Registrator(std::string_view ti, std::string_view actualTi)
                : ActualType::Collector::template ComponentRegistrator<T, ActualType>(ti, actualTi)
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

#    define UNIQUECOMPONENT(Type) DLL_EXPORT_VARIABLE3(, Engine::Plugins::IndexHolder, Type::Registrator<Type>, Engine::Plugins::, _reg, , SINGLE_ARG({ TYPE_INFO(Type), TYPE_INFO(Type) }), Type::T)
#    define UNIQUECOMPONENT2(Type, ext) DLL_EXPORT_VARIABLE3(, Engine::Plugins::IndexHolder, Type::Registrator<Type>, Engine::Plugins::, _reg, ext, SINGLE_ARG({ TYPE_INFO(Type), TYPE_INFO(Type) }), Type::T)
#    define UNIQUECOMPONENT3(Type, ActualType) DLL_EXPORT_VARIABLE3(, Engine::Plugins::IndexHolder, Type::Registrator<ActualType>, Engine::Plugins::, _reg, , SINGLE_ARG({ TYPE_INFO(Type), TYPE_INFO(ActualType) }), Type::T)

#    define VIRTUALUNIQUECOMPONENTBASE(Name)                                                                            \
        DLL_EXPORT_VARIABLE2(, Engine::Plugins::IndexHolder *, Engine::Plugins::, _preg, nullptr, Name) \
        DLL_EXPORT_VARIABLE2(constexpr, const Engine::Plugins::TypeInfo, , typeInfo, TYPE_INFO(Name), Name)

}
}

#else

namespace Engine {
namespace Plugins {
    template <typename _T, typename _Base, typename _VBase>
    struct VirtualComponentImpl : _Base {
        using _Base::_Base;

        using T = _T;
    };

    template <typename _T, typename _Collector, typename _Base>
    struct VirtualComponentBase : _Base {
        using _Base::_Base;

        static bool is_instantiated()
        {
            return true;
        }

        using Collector = _Collector;
        using T = _T;
    };

    template <typename _T, typename _Collector, typename _Base>
    struct Component : _Base {
        using _Base::_Base;

        using Collector = _Collector;
        using T = _T;
    };

}
}

#    define UNIQUECOMPONENT(Type) template Type::Collector::Registry::Annotations::GroupedAnnotation(Engine::type_holder_t<Type::T>, Engine::type_holder_t<Type>);
#    define UNIQUECOMPONENT2(Name, ext)
#    define UNIQUECOMPONENT3(Type, ActualType) template Type::Collector::Registry::Annotations::GroupedAnnotation(Engine::type_holder_t<Type>, Engine::type_holder_t<ActualType>);
#    define VIRTUALUNIQUECOMPONENTBASE(Name)

#endif

namespace Engine {
namespace Plugins {

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
    DLL_EXPORT_VARIABLE2(constexpr, const std::string_view, Engine::Plugins::, _componentName, #Name, FrontendType);

#define NAMED_UNIQUECOMPONENT(Name, Type) \
    COMPONENT_NAME(Name, Type)            \
    UNIQUECOMPONENT(Type)
