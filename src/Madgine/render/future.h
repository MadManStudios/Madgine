#pragma once

namespace Engine {
namespace Render {

    struct RenderFuture {

        RenderFuture() = default;

        RenderFuture(uint64_t value)
            : mValid(true)
            , mValue(value)
        {
        }

        void merge(RenderFuture other)
        {
        }

        explicit operator bool() const
        {
            return mValid;
        }

        uint64_t value() const
        {
            assert(mValid);
            return mValue;
        }

    private:
        bool mValid = false;
        uint64_t mValue;
    };

    template <typename T>
    struct DataFuture {


        RenderFuture mFuture;
        uintptr_t mOffset;
    };

}
}