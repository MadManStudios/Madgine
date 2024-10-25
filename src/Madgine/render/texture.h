#pragma once

#include "texturedescriptor.h"
#include "resourceblock.h"

#include "Meta/math/vector2i.h"

namespace Engine {
namespace Render {

    struct Texture {

        Texture() = default;

        Texture(TextureType type, TextureFormat format, const Vector2i &size = { 0, 0 })
            : mType(type)
            , mFormat(format)
            , mSize(size)
        {
        }

        Texture(Texture &&other)
            : mType(other.mType)
            , mFormat(other.mFormat)
            , mSize(other.mSize)
            , mTextureHandle(std::move(other.mTextureHandle))
            , mResourceBlock(std::move(other.mResourceBlock))
        {
        }

        Texture &operator=(Texture &&other)
        {
            std::swap(mTextureHandle, other.mTextureHandle);
            std::swap(mResourceBlock, other.mResourceBlock);
            std::swap(mType, other.mType);
            std::swap(mFormat, other.mFormat);
            std::swap(mSize, other.mSize);
            return *this;
        }

        TextureHandle handle() const
        {
            return mTextureHandle;
        }

        ResourceBlock resource() const
        {
            return mResourceBlock;
        }

        const Vector2i &size() const
        {
            return mSize;
        }

        TextureFormat format() const
        {
            return mFormat;
        }

        TextureType type() const
        {
            return mType;
        }

    protected:
        UniqueTextureHandle mTextureHandle;
        UniqueResourceBlock mResourceBlock;
        TextureType mType;
        TextureFormat mFormat;
        Vector2i mSize = { 0, 0 };
    };

}
}