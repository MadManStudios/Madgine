#pragma once

namespace Engine {
namespace Reflect {

    struct META_EXPORT ArgumentList {

        using value_type = Value;
        using iterator = std::vector<Reflect::Value>::iterator;
        using const_iterator = std::vector<Reflect::Value>::const_iterator;

        ArgumentList();
        ArgumentList(std::true_type, size_t size);

        template <Concepts::DecayedNoneOf<ArgumentList, std::true_type>... Args>
        explicit ArgumentList(Args &&...args)
            : ArgumentList(std::true_type {}, sizeof...(args))
        {
            size_t i = 0;
            (toValue((*this)[i++], std::forward<Args>(args)), ...);
        }

        ArgumentList(const ArgumentList &other);
        ArgumentList(ArgumentList &&);
        ~ArgumentList();

        ArgumentList &operator=(const ArgumentList &);
        ArgumentList &operator=(ArgumentList &&);

        void clear();

        const Value &operator[](size_t i) const;
        Value &operator[](size_t i);

        Result get(Value &retVal, size_t i) const;

        void reserve(size_t size);
        void resize(size_t size);
        size_t size() const;
        void push_back(Value &&);
        const Value &at(size_t i) const;
        void insert(const_iterator where, const_iterator from, const_iterator to);

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        friend META_EXPORT std::ostream &operator<<(std::ostream &out, const ArgumentList &list);

    private:
        std::vector<Value> mElements;
    };

}
}