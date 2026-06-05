#include "../metalib.h"

#include "value.h"

#include "Generic/execution/execution.h"

#include "metatable.h"
#include "scopeiterator.h"

namespace Engine {
namespace Reflect {

    Value::Value()
    {
    }

    Value::Value(const Value &other)
        : mUnion(other.mUnion)
    {
    }

    Value::Value(Value &&other) noexcept
        : mUnion(std::forward<decltype(other.mUnion)>(other.mUnion))
    {
    }

    Value::~Value()
    {
        clear();
    }

    void Value::clear()
    {
        mUnion = std::monostate {};
    }

    void Value::operator=(const Value &other)
    {
        mUnion = other.mUnion;
    }

    void Value::operator=(Value &&other)
    {
        mUnion = std::move(other.mUnion);
    }

    std::string Value::toShortString() const
    {
        return visit(overloaded {
            [](bool b) {
                return b ? "true"s : "false"s;
            },
            [](const CoWString &s) {
                return "\"" + std::string { s } + "\"";
            },
            [](std::monostate) {
                return "NULL"s;
            },
            [](const ScopePtr &scope) {
                return scope.name();
            },
            [](const OwnedScopePtr &scope) {
                return scope.name();
            },
            [](const Math::Vector2 &v) {
                std::ostringstream ss;
                ss << v;
                return ss.str();
            },
            [](const Math::Vector3 &v) {
                std::ostringstream ss;
                ss << v;
                return ss.str();
            },
            [](const Math::Vector4 &v) {
                std::ostringstream ss;
                ss << v;
                return ss.str();
            },
            [](const Math::Vector2i &v) {
                std::ostringstream ss;
                ss << v;
                return ss.str();
            },
            [](const Math::Vector3i &v) {
                std::ostringstream ss;
                ss << v;
                return ss.str();
            },
            [](const Math::Vector4i &v) {
                std::ostringstream ss;
                ss << v;
                return ss.str();
            },
            [](const Math::Color3 &c) {
                std::ostringstream ss;
                ss << c;
                return ss.str();
            },
            [](const Math::Color4 &c) {
                std::ostringstream ss;
                ss << c;
                return ss.str();
            },
            [](const Math::Quaternion &q) {
                std::ostringstream ss;
                ss << q;
                return ss.str();
            },
            [](const CoW<Math::Matrix3> &) {
                return "Matrix3[...]"s;
            },
            [](const CoW<Math::Matrix4> &) {
                return "Matrix4[...]"s;
            },
            [](const ApiFunction &) {
                return "<api-function>"s;
            },
            [](const BoundApiFunction &) {
                return "<bound-api-function>"s;
            },
            [](const Function &) {
                return "<function>"s;
            },
            [](const SequenceRange &) {
                return "<range>"s;
            },
            [](const AssociativeRange &) {
                return "<map>"s;
            },
            [](const ObjectPtr &) {
                return "<object>"s;
            },
            [](const Enum&e) {
                return std::string { e.toString() };
            },
            [](const Flags&f) {
                std::stringstream ss;
                ss << f;
                return ss.str();
            },
            [](const Reflect::Sender &s) {
                return "<sender>"s;
            },
            [](const Reflect::Binding &b) {
                return "<binding>"s;
            },
            [](const Reflect::ScopeBinding &b) {
                return "<binding>"s;
            },
            [](std::chrono::duration<uint64_t, std::nano> dur) {
                return "<duration>"s;
            },
            [](const ExtendedType &type) {
                return type.toString();
            },
            [](const auto &v) {
                return std::to_string(v);
            } });
    }

    std::string Value::getTypeString() const
    {
        return type().toString();
    }

    Type Value::type() const
    {
        TypeIndex i = index();
        switch (i) {
        case TypeEnum::ScopeValue: {
            const MetaTable *table = as<ScopePtr>().mType;
            return { i, table ? table->mSelf : nullptr };
        }
        case TypeEnum::ScopeBindingValue: {
            const MetaTable *table = as<Reflect::ScopeBinding>().mType;
            return { i, table ? table->mSelf : nullptr };
        }
        default:
            return { i };
        }
    }

    TypeIndex Value::index() const
    {
        return static_cast<TypeEnum>(mUnion.index());
    }

    template <size_t... Is>
    static void setTypeHelper(Value::Union &v, Type type, std::index_sequence<Is...>)
    {
        using Ctor_Type = void (*)(Value::Union &);
        static constexpr Ctor_Type ctors[] = {
            [](Value::Union &v) {
                if constexpr (std::is_default_constructible_v<std::variant_alternative_t<Is, Value::Union>>)
                    v.emplace<Is>();
                else
                    throw 0;
            }...
        };
        ctors[static_cast<unsigned char>(type.mType.mIndex)](v);
    }

    void Value::setType(Type type)
    {
        if (this->type() != type) {
            setTypeHelper(mUnion, type, std::make_index_sequence<std::variant_size_v<Union>>());
            switch (type.mType) {
            case TypeEnum::ScopeValue:
                std::get<ScopePtr>(mUnion).mType = *type.mSecondary.mMetaTable;
                break;
            case TypeEnum::ApiFunctionValue:
                std::get<ApiFunction>(mUnion).mTable = *type.mSecondary.mFunctionTable;
                break;
            case TypeEnum::OwnedScopeValue:
                std::get<OwnedScopePtr>(mUnion).construct(*type.mSecondary.mMetaTable);
                break;
            default:
                break;
            }
        }
    }

    Reflect::Result Value::call(Value &retVal, const ArgumentList &args) const
    {
        return std::visit(overloaded {
                              [&](const ApiFunction &function) {
                                  return function(retVal, args);
                              },
                              [&](const Reflect::Function &function) {
                                  return function(retVal, args);
                              },
                              [&](const BoundApiFunction &function) {
                                  return function(retVal, args);
                              },
                              [&](const ScopePtr &scope) {
                                  return scope.call(retVal, args);
                              },
                              [&](const OwnedScopePtr &scope) {
                                  return scope.get().call(retVal, args);
                              },
                              [&](const ObjectPtr &o) {
                                  return o.call(retVal, args);
                              },
                              [](const auto &) -> Reflect::Result {
                                  throw "calling operator is not supported";
                              } },
            mUnion);
    }

    ScopeIterator Value::end() const
    {
        return { *this, nullptr };
    }

    DERIVE_OPERATOR(StreamOut, <<)

    std::ostream &operator<<(std::ostream &stream,
        const Value &v)
    {
        stream << "<" << v.getTypeString() << ">";

        v.visit([&](const auto &v) {
            if constexpr (has_operator_StreamOut<decltype(v), std::ostream &> || has_operator_StreamOut<std::ostream &, decltype(v)>)
                stream << std::forward<decltype(v)>(v);
        });

        return stream;
    }

}
}
