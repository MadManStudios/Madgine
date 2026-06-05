#include "../opengllib.h"

#include "opengltexture.h"

#include "Generic/areaview.h"
#include "Generic/bytebuffer.h"
#include "Generic/functor.h"

namespace Engine {
namespace Render {

    OpenGLTexture::OpenGLTexture(TextureType type, TextureFormat format, Math::Vector2i size, size_t samples, const Memory::ByteBuffer &data)
        : Texture(type, format, size)
        , mSamples(samples)
    {
        glGenTextures(1, &mHandle);
        GL_CHECK();
        if (mType != TextureType_2DMultiSample)
            setFilter(GL_LINEAR);
        bind();
#if !OPENGL_ES
        if (mType != TextureType_2DMultiSample) {
            glTexParameteri(target(), GL_TEXTURE_BASE_LEVEL, 0);
            GL_CHECK();
            glTexParameteri(target(), GL_TEXTURE_MAX_LEVEL, 0);
            GL_CHECK();
        }
#endif

        mBlock.mResources[0] = ConstTexturePtr { this, NoOpFunctor {} };
        mResourceBlock.setupAs<OpenGLResourceBlock<1> *>() = &mBlock;

        GLenum internalStorage;
        GLenum internalFormat;
        GLenum sizedFormat;
        switch (mFormat) {
        case FORMAT_RGBA8:
            internalStorage = GL_UNSIGNED_BYTE;
            internalFormat = GL_RGBA;
            sizedFormat = GL_RGBA8;
            break;
        case FORMAT_RGBA8_SRGB:
            internalStorage = GL_UNSIGNED_BYTE;
            internalFormat = GL_RGBA;
            sizedFormat = GL_SRGB8_ALPHA8;
            break;
        case FORMAT_RGBA16F:
            internalStorage = GL_FLOAT;
            internalFormat = GL_RGBA;
            sizedFormat = GL_RGBA16F;
            assert(!data.mData);
            break;
        case FORMAT_R32F:
            internalStorage = GL_FLOAT,
            internalFormat = GL_RED;
            sizedFormat = GL_R32F;
            break;
        case FORMAT_D24:
            internalStorage = GL_UNSIGNED_INT;
            internalFormat = GL_DEPTH_COMPONENT;
            sizedFormat = GL_DEPTH_COMPONENT24;
            assert(!data.mData);
            break;
        case FORMAT_D32:
            internalStorage = GL_FLOAT;
            internalFormat = GL_DEPTH_COMPONENT;
            sizedFormat = GL_DEPTH_COMPONENT32F;
            assert(!data.mData);
            break;
        default:
            throw 0;
        }
        bind();
        switch (mType) {
        case TextureType_2D:
            glTexImage2D(target(), 0, sizedFormat, size.x, size.y, 0, internalFormat, internalStorage, data.mData);
            break;
#if MULTISAMPLING
        case TextureType_2DMultiSample:
            glTexStorage2DMultisample(target(), mSamples, sizedFormat, size.x, size.y, true);
            break;
#endif
        case TextureType_Cube:
            for (int i = 0; i < 6; ++i)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, sizedFormat, size.x, size.y, 0, internalFormat, internalStorage, data.mData);
            break;
        default:
            throw 0;
        }
        GL_CHECK();
    }

    OpenGLTexture::OpenGLTexture(TextureType type, TextureFormat format, size_t samples)
        : Texture(type, format)
        , mSamples(samples)
    {
    }

    OpenGLTexture::OpenGLTexture(OpenGLTexture &&other)
        : Texture(std::move(other))
        , mSamples(std::move(other.mSamples))
        , mBlock(std::move(other.mBlock))        
    {
        if (mResourceBlock) {
            [[maybe_unused]] OpenGLResourceBlock<> *block = mResourceBlock.release<OpenGLResourceBlock<> *>();
            assert(block == &other.mBlock);
            mResourceBlock.setupAs<OpenGLResourceBlock<> *>() = &mBlock;
        }
        if (other.mResourceBlock) {
            [[maybe_unused]] OpenGLResourceBlock<> *block = other.mResourceBlock.release<OpenGLResourceBlock<> *>();
            assert(block == &mBlock);
            other.mResourceBlock.setupAs<OpenGLResourceBlock<> *>() = &other.mBlock;
        }
    }

    OpenGLTexture::~OpenGLTexture()
    {
        reset();
    }

    OpenGLTexture &OpenGLTexture::operator=(OpenGLTexture &&other)
    {
        Texture::operator=(std::move(other));
        std::swap(mBlock, other.mBlock);
        if (mResourceBlock) {
            [[maybe_unused]] OpenGLResourceBlock<> *block = mResourceBlock.release<OpenGLResourceBlock<> *>();
            assert(block == &other.mBlock);
            mResourceBlock.setupAs<OpenGLResourceBlock<> *>() = &mBlock;
        }
        if (other.mResourceBlock) {
            [[maybe_unused]] OpenGLResourceBlock<> *block = other.mResourceBlock.release<OpenGLResourceBlock<> *>();
            assert(block == &mBlock);
            other.mResourceBlock.setupAs<OpenGLResourceBlock<> *>() = &other.mBlock;
        }

        std::swap(mSamples, other.mSamples);
        return *this;
    }

    void OpenGLTexture::reset()
    {
        if (mHandle) {
            glDeleteTextures(1, &mHandle);
            GL_CHECK();
            mHandle = 0;
        }
        if (mResourceBlock) {
            [[maybe_unused]] OpenGLResourceBlock<> *block = mResourceBlock.release<OpenGLResourceBlock<> *>();
            assert(block == &mBlock);
        }
    }

    void OpenGLTexture::bind() const
    {
        glBindTexture(target(), mHandle);
        GL_CHECK();
    }

    GLuint OpenGLTexture::handle() const
    {
        return mHandle;
    }

    void OpenGLTexture::setSubData(Math::Vector2i offset, Math::Vector2i size, const Memory::ByteBuffer &data)
    {
        GLenum internalStorage;
        GLenum internalFormat;
        switch (mFormat) {
        case FORMAT_RGBA8:
            internalStorage = GL_UNSIGNED_BYTE;
            internalFormat = GL_RGBA;
            break;
        case FORMAT_RGBA8_SRGB:
            internalStorage = GL_UNSIGNED_BYTE;
            internalFormat = GL_RGBA;
            break;
        case FORMAT_RGBA16F:
            internalStorage = GL_FLOAT;
            internalFormat = GL_RGBA;
            break;
        case FORMAT_R32F:
            internalStorage = GL_FLOAT,
            internalFormat = GL_RED;
            break;
        case FORMAT_D24:
            internalStorage = GL_UNSIGNED_INT;
            internalFormat = GL_DEPTH_COMPONENT;
            break;
        default:
            throw 0;
        }
        bind();
        glTexSubImage2D(target(), 0, offset.x, offset.y, size.x, size.y, internalFormat, internalStorage, data.mData);
        GL_CHECK();
    }

    void OpenGLTexture::setWrapMode(GLint mode)
    {
        bind();
        glTexParameteri(target(), GL_TEXTURE_WRAP_S, mode);
        GL_CHECK();
        glTexParameteri(target(), GL_TEXTURE_WRAP_T, mode);
        GL_CHECK();
    }

    void OpenGLTexture::setFilter(GLint filter)
    {
        assert(mType != TextureType_2DMultiSample);
        bind();
        glTexParameteri(target(), GL_TEXTURE_MIN_FILTER, filter);
        GL_CHECK();
        glTexParameteri(target(), GL_TEXTURE_MAG_FILTER, filter);
        GL_CHECK();
    }

    GLenum OpenGLTexture::target() const
    {
        switch (mType) {
        case TextureType_2D:
            return GL_TEXTURE_2D;
#if MULTISAMPLING
        case TextureType_2DMultiSample:
            return GL_TEXTURE_2D_MULTISAMPLE;
#endif
        case TextureType_Cube:
            return GL_TEXTURE_CUBE_MAP;
        default:
            throw 0;
        }
    }

}
}
