#pragma once

namespace Engine {

template <auto Builder, auto Init>
struct DynamicAccessorList {

    void init() {
        Init(mData);
    }

    constexpr const Accessor* data() const {
        return mData.data();
    }

    decltype(Builder()) mData = Builder();
};

}