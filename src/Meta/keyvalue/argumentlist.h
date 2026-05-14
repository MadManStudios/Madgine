#pragma once

namespace Engine {

struct META_EXPORT ArgumentList {

    using value_type = ValueType;

    ArgumentList();
    ArgumentList(std::true_type, size_t size);

    template <DecayedNoneOf<ArgumentList, std::true_type>... Args>
    explicit ArgumentList(Args &&...args)
        : ArgumentList(std::true_type {}, sizeof...(args))
    {
        size_t i = 0;
        (to_ValueType((*this)[i++], std::forward<Args>(args)), ...);
    }

    ArgumentList(const ArgumentList &other);
    ArgumentList(ArgumentList &&);
    ~ArgumentList();

    ArgumentList &operator=(const ArgumentList &);
    ArgumentList &operator=(ArgumentList &&);

    void clear();

    const ValueType &operator[](size_t i) const;
    ValueType &operator[](size_t i);

    void reserve(size_t size);
    void resize(size_t size);
    size_t size() const;
    void push_back(ValueType &&);
    const ValueType &at(size_t i) const;
    void insert(std::vector<ValueType>::const_iterator where, std::vector<ValueType>::const_iterator from, std::vector<ValueType>::const_iterator to);

    std::vector<ValueType>::iterator begin();
    std::vector<ValueType>::iterator end();
    std::vector<ValueType>::const_iterator begin() const;
    std::vector<ValueType>::const_iterator end() const;

    friend META_EXPORT std::ostream &operator<<(std::ostream &out, const ArgumentList &list);

private:
    std::vector<ValueType> mElements;
};

}