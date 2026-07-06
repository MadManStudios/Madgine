#pragma once

namespace Engine {
namespace Serialize {

    template <typename T>
    struct Replicated {

        Replicated() = default;
        Replicated(const T &value)
            : mValue(value)
        {
        }

        const T &get() const
        {
            return mValue;
        }

        Replicated<T> &operator=(const T &value)
        {
            if (mValue != value) {
                mValue = value;
                mDirty = true;
            }
            return *this;
        }

        bool isDirty() const
        {
            return mDirty;
        }

        void clearDirty()
        {
            mDirty = false;
        }

    private:
        T mValue;
        bool mDirty = false;
    };

}
}