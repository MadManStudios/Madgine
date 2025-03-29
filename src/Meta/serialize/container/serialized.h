#pragma once

namespace Engine {
namespace Serialize {
    //TODO: Implement OffsetPtr interface + operations
    template <typename T>
    struct Serialized : SerializableBase {
        template <typename... _Ty>
        Serialized(_Ty &&... args)
            : mData(std::forward<_Ty>(args)...)
        {
            //this->setParent(mData, unit());
        }

        template <typename Ty>
        void operator=(Ty &&v)
        {
            mData = std::forward<Ty>(v);
        }

        T &operator*()
        {
            return mData;
        }

        const T &operator*() const
        {
            return mData;
        }

        T *operator->()
        {
            return &mData;
        }

        const T *operator->() const
        {
            return &mData;
        }

        T *ptr()
        {
            return &mData;
        }

        const T *ptr() const
        {
            return &mData;
        }

        operator const T &() const
        {
            return mData;
        }

        template <typename Ty>
        void operator+=(Ty &&v)
        {
            mData += std::forward<Ty>(v);
        }

        template <typename Ty>
        void operator-=(Ty &&v)
        {
            mData -= std::forward<Ty>(v);
        }

        StreamResult readState(FormattedSerializeStream &in, const char *name = nullptr)
        {
            return read(in, mData, name);
        }

        void writeState(FormattedSerializeStream &out, const char *name = nullptr) const
        {
            write(out, mData, name);
        }

        void setActive(bool active, bool existenceChanged)
        {
            Serialize::setActive(mData, active, existenceChanged);
        }

    private:
        T mData;
    };

}
}
