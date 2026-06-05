#pragma once

#include "Generic/indextype.h"

namespace Engine {
namespace Plugins {

    struct IndexHolder {

        IndexHolder(size_t index, IndexType<size_t> &baseIndex)
            : mIndex(index)
            , mBaseIndex(baseIndex)
        {
        }

        ~IndexHolder()
        {
            mIndex.reset();
        }

        size_t index() const
        {
            assert(isValid());
            return mIndex + mBaseIndex;
        }

        bool isValid() const
        {
            return mIndex && mBaseIndex;
        }

    protected:
        IndexType<size_t> mIndex;
        // Make it a member to prevent problems through weak symbols during link time
        IndexType<size_t> &mBaseIndex;
    };

}
}