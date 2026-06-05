#pragma once

namespace Engine {
namespace Reflect {

    struct META_EXPORT ArgumentList {

        using value_type = Value;

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
        void insert(std::vector<Value>::const_iterator where, std::vector<Value>::const_iterator from, std::vector<Value>::const_iterator to);

        std::vector<Value>::iterator begin();
        std::vector<Value>::iterator end();
        std::vector<Value>::const_iterator begin() const;
        std::vector<Value>::const_iterator end() const;

        friend META_EXPORT std::ostream &operator<<(std::ostream &out, const ArgumentList &list);

    private:
        std::vector<Value> mElements;
    };

}
}