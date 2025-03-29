#include "../metalib.h"
#include "valuetype.h"

#include "metatable.h"

#include "Generic/execution/execution.h"

namespace Engine {

ValueType::ValueType()
{
}

ValueType::ValueType(const ValueType &other)
    : mUnion(other.mUnion)
{
}

ValueType::ValueType(ValueType &&other) noexcept
    : mUnion(std::forward<decltype(other.mUnion)>(other.mUnion))
{
}

ValueType::~ValueType()
{
    clear();
}

void ValueType::clear()
{
    mUnion = std::monostate {};
}

void ValueType::operator=(const ValueType &other)
{
    mUnion = other.mUnion;
}

void ValueType::operator=(ValueType &&other)
{
    mUnion = std::move(other.mUnion);
}

std::string ValueType::toShortString() const
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
        [](const Vector2 &v) {
            std::ostringstream ss;
            ss << v;
            return ss.str();
        },
        [](const Vector3 &v) {
            std::ostringstream ss;
            ss << v;
            return ss.str();
        },
        [](const Vector4 &v) {
            std::ostringstream ss;
            ss << v;
            return ss.str();
        },
        [](const Vector2i &v) {
            std::ostringstream ss;
            ss << v;
            return ss.str();
        },
        [](const Vector3i &v) {
            std::ostringstream ss;
            ss << v;
            return ss.str();
        },
        [](const Vector4i &v) {
            std::ostringstream ss;
            ss << v;
            return ss.str();
        },
        [](const Color3 &c) {
            std::ostringstream ss;
            ss << c;
            return ss.str();
        },
        [](const Color4 &c) {
            std::ostringstream ss;
            ss << c;
            return ss.str();
        },
        [](const Quaternion &q) {
            std::ostringstream ss;
            ss << q;
            return ss.str();
        },
        [](const CoW<Matrix3> &) {
            return "Matrix3[...]"s;
        },
        [](const CoW<Matrix4> &) {
            return "Matrix4[...]"s;
        },
        [](const ApiFunction &) {
            return "<api-function>"s;
        },
        [](const BoundApiFunction &) {
            return "<bound-api-function>"s;
        },
        [](const KeyValueFunction &) {
            return "<function>"s;
        },
        [](const KeyValueVirtualSequenceRange &) {
            return "<range>"s;
        },
        [](const KeyValueVirtualAssociativeRange &) {
            return "<map>"s;
        },
        [](const ObjectPtr &) {
            return "<object>"s;
        },
        [](const EnumHolder &e) {
            return std::string { e.toString() };
        },
        [](const FlagsHolder &f) {
            std::stringstream ss;
            ss << f;
            return ss.str();
        },
        [](const KeyValueSender &s) {
            return "<sender>"s;
        },
        [](std::chrono::duration<uint64_t, std::nano> dur) {
            return "<duration>"s;
        },
        [](const ExtendedValueTypeDesc &type) {
            return type.toString();
        },
        [](const auto &v) {
            return std::to_string(v);
        } });
}

std::string ValueType::getTypeString() const
{
    return type().toString();
}

bool ValueType::isReference() const
{
    return visit(overloaded {
        [](const ScopePtr &) {
            return true;
        },
        [](const KeyValueVirtualSequenceRange &range) {
            return range.isReference();
        },
        [](const KeyValueVirtualAssociativeRange &range) {
            return range.isReference();
        },
        [](const auto &) {
            return false;
        } });
}

ValueTypeDesc ValueType::type() const
{
    ValueTypeIndex i = index();
    switch (i) {
    case ValueTypeEnum::ScopeValue: {
        const MetaTable *table = as<ScopePtr>().mType;
        return { i, table ? table->mSelf : nullptr };
    }
    default:
        return { i };
    }
}

ValueTypeIndex ValueType::index() const
{
    return static_cast<ValueTypeEnum>(mUnion.index());
}

template <size_t... Is>
static void setTypeHelper(ValueType::Union &v, ValueTypeDesc type, std::index_sequence<Is...>)
{
    using Ctor_Type = void (*)(ValueType::Union &);
    static constexpr Ctor_Type ctors[] = {
        [](ValueType::Union &v) {
            if constexpr (std::is_default_constructible_v<std::variant_alternative_t<Is, ValueType::Union>>)
                v.emplace<Is>();
            else
                throw 0;
        }...
    };
    ctors[static_cast<unsigned char>(type.mType.mIndex)](v);
}

void ValueType::setType(ValueTypeDesc type)
{
    if (this->type() != type) {
        setTypeHelper(mUnion, type, std::make_index_sequence<std::variant_size_v<Union>>());
        switch (type.mType) {
        case ValueTypeEnum::ScopeValue:
            std::get<ScopePtr>(mUnion).mType = *type.mSecondary.mMetaTable;
            break;
        case ValueTypeEnum::ApiFunctionValue:
            std::get<ApiFunction>(mUnion).mTable = *type.mSecondary.mFunctionTable;
            break;
        case ValueTypeEnum::OwnedScopeValue:
            std::get<OwnedScopePtr>(mUnion).construct(*type.mSecondary.mMetaTable);
            break;
        default:
            break;
        }
    }
}

void ValueType::call(ValueType &retVal, const ArgumentList &args) const
{
    return std::visit(overloaded {
                          [&](const ApiFunction &function) {
                              return function(retVal, args);
                          },
                          [&](const KeyValueFunction &function) {
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
                          [](const auto &) {
                              throw "calling operator is not supported";
                          } },
        mUnion);
}

DERIVE_OPERATOR(StreamOut, <<)

std::ostream &operator<<(std::ostream &stream,
    const Engine::ValueType &v)
{
    stream << "<" << v.getTypeString() << ">";

    v.visit([&](const auto &v) {
        if constexpr (has_operator_StreamOut<decltype(v), std::ostream &> || has_operator_StreamOut<std::ostream &, decltype(v)>)
            stream << std::forward<decltype(v)>(v);
    });

    return stream;
}
}
