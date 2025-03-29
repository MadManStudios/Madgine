#pragma once

namespace Engine {

struct META_EXPORT ObjectInstance {
    virtual ~ObjectInstance() = default;

    virtual bool getValue(ValueType &retVal, std::string_view name) const = 0;
    virtual void setValue(std::string_view name, const ValueType &value) = 0;
    virtual std::map<std::string_view, ValueType> values() const;

    virtual void call(ValueType &retVal, const ArgumentList &args);

    virtual std::string descriptor() const;
};

}