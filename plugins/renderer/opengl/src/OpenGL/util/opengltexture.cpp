#include "../opengllib.h"

#include "opengltexture.h"

#include "Generic/areaview.h"
#include "Generic/bytebuffer.h"

namespace Engine {
namespace Render {

    OpenGLTexture::OpenGLTexture(TextureType type, TextureFormat format, Vector2i size, size_t samples, const ByteBuffer &data)
        : Texture(type, format, size)
        , mSamples(samples)
    {
        GLuint temp;
        glGenTextures(1, &mTextureHandle.setupAs<GLuint>());
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

        mBlock.mResources[0].mHandle = mTextureHandle;
        mBlock.mResources[0].mTarget = target();
        mResourceBlock.setupAs<OpenGLResourceBlock<1> *>() = &mBlock;

        GLenum internalStorage;
        GLenum internalFormat;
        GLenum sizedFormat;
        size_t byteWidth;
        switch (mFormat) {
        case FORMAT_RGBA8:
            internalStorage = GL_UNSIGNED_BYTE;
            internalFormat = GL_RGBA;
            sizedFormat = GL_RGBA8;
            byteWidth = 4;
            break;
        case FORMAT_RGBA8_SRGB:
            internalStorage = GL_UNSIGNED_BYTE;
            internalFormat = GL_RGBA;
            sizedFormat = GL_SRGB8_ALPHA8;
            byteWidth = 4;
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
        , mBlock(std::move(other.mBlock))
        , mSamples(std::move(other.mSamples))
    {
        if (mResourceBlock) {
            OpenGLResourceBlock<> *block = mResourceBlock.release<OpenGLResourceBlock<> *>();
            assert(block == &other.mBlock);
            mResourceBlock.setupAs<OpenGLResourceBlock<> *>() = &mBlock;
        }
        if (other.mResourceBlock) {
            OpenGLResourceBlock<> *block = other.mResourceBlock.release<OpenGLResourceBlock<> *>();
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
            OpenGLResourceBlock<> *block = mResourceBlock.release<OpenGLResourceBlock<> *>();
            assert(block == &other.mBlock);
            mResourceBlock.setupAs<OpenGLResourceBlock<> *>() = &mBlock;
        }
        if (other.mResourceBlock) {
            OpenGLResourceBlock<> *block = other.mResourceBlock.release<OpenGLResourceBlock<> *>();
            assert(block == &mBlock);
            other.mResourceBlock.setupAs<OpenGLResourceBlock<> *>() = &other.mBlock;
        }

        std::swap(mSamples, other.mSamples);
        return *this;
    }

    void OpenGLTexture::reset()
    {
        if (mTextureHandle) {
            GLuint handle = mTextureHandle.release<GLuint>();
            glDeleteTextures(1, &handle);
            GL_CHECK();
        }
        if (mResourceBlock) {
            OpenGLResourceBlock<> *block = mResourceBlock.release<OpenGLResourceBlock<> *>();
            assert(block == &mBlock);
        }
    }

    void OpenGLTexture::bind() const
    {
        glBindTexture(target(), mTextureHandle);
        GL_CHECK();
    }

    void OpenGLTexture::setData(Vector2i size, const ByteBuffer &data)
    {
        *this = OpenGLTexture { mType, mFormat, size, mSamples, data };
    }

    void OpenGLTexture::setSubData(Vector2i offset, Vector2i size, const ByteBuffer &data)
    {
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
            sizedFormat = GL_SRGB;
            break;
        case FORMAT_RGBA16F:
            internalStorage = GL_FLOAT;
            internalFormat = GL_RGBA;
            sizedFormat = GL_RGBA16F;
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
