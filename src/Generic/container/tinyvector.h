#pragma once

#include "../bits/ptr.h"
#include "../bits/variant.h"

namespace Engine {

template <typename T>
struct TinyVector {

    TinyVector() = default;
    TinyVector(std::initializer_list<T> data)
    {
        if (data.size() == 0) {
            mData.template emplace<0>();
        } else if (data.size() == 1) {
            mData.template emplace<1>(std::move(*data.begin()));
        } else {
            mData.template emplace<2>(std::make_unique<std::vector<T>>(std::move(data)));
        }
    }

    TinyVector(const TinyVector &other)
    {
        (*this) = other;
    }

    TinyVector& operator=(const TinyVector& other) {
        other.mData.visit(overloaded {
            [this](std::monostate) {
                mData.template emplace<0>();
            },
            [this](const T &t) {
                mData.template emplace<1>(t);
            },
            [this](const BitUniquePtr<std::vector<T>> &p) {
                if (p->empty()) {
                    mData.template emplace<0>();
                } else if (p->size() == 1) {
                    mData.template emplace<1>(p->front());
                } else {
                    mData.template emplace<2>(std::make_unique<std::vector<T>>(*p));
                }
            } });
        return *this;
    }

    bool empty() const
    {
        return mData.visit(overloaded {
            [](std::monostate) {
                return true;
            },
            [](const T &) {
                return false;
            },
            [](const BitUniquePtr<std::vector<T>> &v) {
                return v->empty();
            } });
    }

    const T *begin() const
    {
        return mData.visit(overloaded {
            [](std::monostate) -> const T * {
                return nullptr;
            },
            [](const T &t) {
                return &t;
            },
            [](const BitUniquePtr<std::vector<T>> &v) -> const T * {
                return v->data();
            } });
    }

    const T *end() const
    {
        return mData.visit(overloaded {
            [](std::monostate) -> const T * {
                return nullptr;
            },
            [](const T &t) {
                return &t + 1;
            },
            [](const BitUniquePtr<std::vector<T>> &v) -> const T * {
                return v->data() + v->size();
            } });
    }

    T operator[](size_t i) const
    {
        return mData.visit(overloaded {
            [](std::monostate) {
                throw 0;
            },
            [=](const T &t) {
                if (i != 0)
                    throw 0;
                return t;
            },
            [=](const BitUniquePtr<std::vector<T>> &v) {
                return (*v)[i];
            } });
    }

    void push_back(T &&t)
    {
        mData.visit(overloaded {
            [&](std::monostate) {
                mData.emplace<1>(std::forward<T>(t));
            },
            [&](T &first) {
                mData.emplace<2>(std::make_unique<std::vector<T>>({ std::move(first), std::forward<T>(t) }));
            },
            [=](BitUniquePtr<std::vector<T>> &v) {
                v->push_back(std::forward<T>(t));
            } });
    }

private:
    BitVariant<std::monostate, T, BitUniquePtr<std::vector<T>>> mData;
};
}