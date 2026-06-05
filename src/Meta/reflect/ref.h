#pragma once

namespace Engine {
namespace Reflect {

    struct ValueRef {

        ValueRef(Value &ref)
            : mRef(ref)
        {
        }

        operator Value &()
        {
            return mRef;
        }

        template <typename T>
        ValueRef &operator=(T &&v)
        {
            toValue(mRef, std::forward<T>(v));
            return *this;
        }

    private:
        Value &mRef;
    };

}
}