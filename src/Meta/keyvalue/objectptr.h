#pragma once


namespace Engine {

struct META_EXPORT ObjectPtr {

    ObjectPtr() = default;
    ObjectPtr(std::monostate);
    ObjectPtr(const std::shared_ptr<ObjectInstance> &instance);

    void setValue(std::string_view name, const ValueType &value);
    bool getValue(ValueType &retVal, std::string_view name) const;
    std::map<std::string_view, ValueType> values() const;

    void call(ValueType &retVal, const ArgumentList &args) const;

    std::string descriptor() const;

    void reset();
    ObjectInstance *get();
    const ObjectInstance *get() const;

    explicit operator bool() const;

    bool operator==(const ObjectPtr &other) const;

private:
    std::shared_ptr<ObjectInstance> mInstance;
};
}

