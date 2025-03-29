#include "opengllib.h"

#include "openglrendertexture.h"

#include "Madgine/render/rendertextureconfig.h"

#include "Generic/bytebuffer.h"

#include "openglrendercontext.h"

#include "Meta/math/matrix4.h"

namespace Engine {
namespace Render {

    GLenum toGLFormat(TextureFormat format)
    {
        switch (format) {
        case FORMAT_RGBA8:
            return GL_RGBA8;
        case FORMAT_RGBA16F:
            return GL_RGBA16F;
        case FORMAT_R32F:
            return GL_R32F;
        default:
            throw 0;
        }
    }

    void attachFramebufferTexture(GLenum attachment, const OpenGLTexture &tex, size_t j)
    {
#if CUBE_FRAMEBUFFER
#    if !OPENGL_ES
        if (glFramebufferTexture) {
#    endif
            glFramebufferTexture(GL_FRAMEBUFFER, attachment, tex.handle(), 0);
            GL_CHECK();
#    if !OPENGL_ES
        } else
#    endif
#endif
#if OPENGL_ES < 32
        {
            GLenum target = tex.target();
            if (target == GL_TEXTURE_CUBE_MAP) {
                target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + j;
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, tex.handle(), 0);
            GL_CHECK();
        }
#endif
    }

    OpenGLRenderTexture::OpenGLRenderTexture(OpenGLRenderContext *context, const Vector2i &size, const RenderTextureConfig &config)
        : OpenGLRenderTarget(context, false, config.mName, config.mFlipFlop, config.mBlitSource)
        , mSamples(context->supportsMultisampling() ? config.mSamples : 1)
        , mSize { 0, 0 }
        , mType(config.mType)
        , mCreateDepthTexture(config.mCreateDepthBufferView)
    {
        if (mType == TextureType_2DMultiSample && !context->supportsMultisampling())
            mType = TextureType_2D;

        bool createDepthBufferView = config.mCreateDepthBufferView;

        size_t bufferCount = config.mFlipFlop ? 2 : 1;
        mFramebufferCount = getFramebufferCount(&createDepthBufferView);

        for (size_t i = 0; i < config.mTextureCount * bufferCount; ++i) {
            OpenGLTexture &tex = mTextures.emplace_back(mType, config.mFormat, config.mSamples);
        }

        if (mCreateDepthTexture) {
            mDepthTexture = { mType, FORMAT_D32, mSamples };
        } else {
            glGenRenderbuffers(1, &mDepthRenderbuffer);
            GL_CHECK();
        }

        resize(size);
    }

    OpenGLRenderTexture::~OpenGLRenderTexture()
    {
        for (auto &[offsets, framebuffers] : mFramebuffers)
            for (GLuint framebuffer : framebuffers)
                if (framebuffer != 0)
                    glDeleteFramebuffers(1, &framebuffer);
        GL_CHECK();
        if (mDepthRenderbuffer) {
            glDeleteRenderbuffers(1, &mDepthRenderbuffer);
            GL_CHECK();
        }
    }

    bool OpenGLRenderTexture::resizeImpl(const Vector2i &size)
    {
        if (mSize == size)
            return false;

        mSize = size;

        GLsizei width = static_cast<GLsizei>(size.x);
        GLsizei height = static_cast<GLsizei>(size.y);

        assert(width > 0 && height > 0);

        for (OpenGLTexture &tex : mTextures) {
            tex.setData({ width, height }, {});
            if (mType != TextureType_2DMultiSample) {
                tex.setWrapMode(GL_CLAMP_TO_EDGE);
                tex.setFilter(GL_NEAREST);
            }
        }

        if (mDepthRenderbuffer) {
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbuffer);
            GL_CHECK();

            if (mSamples > 1)
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, mSamples, GL_DEPTH_COMPONENT24, width, height);
            else
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
            GL_CHECK();

            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            GL_CHECK();
        }
        if (mCreateDepthTexture) {
            mDepthTexture.setData({ width, height }, {});
            if (mType == TextureType_Cube) {
                mDepthTexture.setWrapMode(GL_CLAMP_TO_EDGE);
                mDepthTexture.setFilter(GL_NEAREST);
            }
        }

        size_t framebufferCount = getFramebufferCount();
        for (auto &[offsets, framebuffers] : mFramebuffers) {
            for (size_t j = 0; j < framebufferCount; ++j) {

                glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[j]);
                GL_CHECK();
                for (size_t i = 0; i < textureCount(); ++i) {
                    const OpenGLTexture &tex = mTextures[offsets[i] * textureCount() + i];
                    attachFramebufferTexture(GL_COLOR_ATTACHMENT0 + i, tex, i);
                }

                if (mCreateDepthTexture) {
                    attachFramebufferTexture(GL_DEPTH_ATTACHMENT, mDepthTexture, 0);
                } else {
                    glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbuffer);
                    GL_CHECK();
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderbuffer);
                    GL_CHECK();
                }
            }
        }

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        GL_CHECK();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        GL_CHECK();

        return true;
    }

    void OpenGLRenderTexture::beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        auto pib = mFramebuffers.try_emplace(mFlipFlopIndices);
        auto it = pib.first;
        if (pib.second) {
            glGenFramebuffers(mFramebufferCount, it->second.data());
            GL_CHECK();

            for (size_t j = 0; j < mFramebufferCount; ++j) {

                glBindFramebuffer(GL_FRAMEBUFFER, it->second[j]);
                GL_CHECK();

                for (size_t i = 0; i < textureCount(); ++i) {
                    const OpenGLTexture &tex = mTextures[mFlipFlopIndices[i] * textureCount() + i];
                    attachFramebufferTexture(GL_COLOR_ATTACHMENT0 + i, tex, i);
                }

                if (mCreateDepthTexture) {
                    attachFramebufferTexture(GL_DEPTH_ATTACHMENT, mDepthTexture, 0);
                } else {
                    glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbuffer);
                    GL_CHECK();
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderbuffer);
                    GL_CHECK();
                }

                GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
                assert(textureCount() <= array_size(DrawBuffers));
                glDrawBuffers(textureCount(), DrawBuffers);
                GL_CHECK();

                if (GLenum check = glCheckFramebufferStatus(GL_FRAMEBUFFER); check != GL_FRAMEBUFFER_COMPLETE) {
                    if (check == 0) {
                        GL_CHECK();
                    } else {
                        LOG_ERROR("Incomplete Framebuffer Status: " << check);
                        glDump();
                    }
                    std::terminate();
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                GL_CHECK();
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, it->second[targetSubresourceIndex]);
        GL_CHECK();

        OpenGLRenderTarget::beginIteration(targetIndex, targetCount, targetSubresourceIndex);
    }

    void OpenGLRenderTexture::endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const
    {
        OpenGLRenderTarget::endIteration(targetIndex, targetCount, targetSubresourceIndex);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        GL_CHECK();
    }

    void OpenGLRenderTexture::beginFrame()
    {
        auto it = mFramebuffers.find(mFlipFlopIndices);
        if (it != mFramebuffers.end()) {
            glBindFramebuffer(GL_FRAMEBUFFER, it->second[0]);
            GL_CHECK();
        }

        OpenGLRenderTarget::beginFrame();

        if (mBlitSource)
            blit(mBlitSource);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        GL_CHECK();
    }

    RenderFuture OpenGLRenderTexture::endFrame()
    {
        return OpenGLRenderTarget::endFrame();
    }

    const OpenGLTexture *OpenGLRenderTexture::texture(size_t index) const
    {
        int offset = canFlipFlop() ? mFlipFlopIndices[index] : 0;
        return &mTextures[textureCount() * offset + index];
    }

    size_t OpenGLRenderTexture::textureCount() const
    {
        int bufferCount = canFlipFlop() ? 2 : 1;
        return mTextures.size() / bufferCount;
    }

    const OpenGLTexture *OpenGLRenderTexture::depthTexture() const
    {
        return &mDepthTexture;
    }

    void OpenGLRenderTexture::blit(RenderTarget *input) const
    {
        OpenGLRenderTexture *inputTex = dynamic_cast<OpenGLRenderTexture *>(input);
        assert(inputTex);

        size_t count = std::min(textureCount(), inputTex->textureCount());

        auto pib = mFramebuffers.try_emplace(mFlipFlopIndices);
        auto it = pib.first;
        if (pib.second) {
            glGenFramebuffers(mFramebufferCount, it->second.data());
            GL_CHECK();

            for (size_t j = 0; j < mFramebufferCount; ++j) {

                glBindFramebuffer(GL_FRAMEBUFFER, it->second[j]);
                GL_CHECK();

                for (size_t i = 0; i < textureCount(); ++i) {
                    const OpenGLTexture &tex = mTextures[mFlipFlopIndices[i] * textureCount() + i];
                    attachFramebufferTexture(GL_COLOR_ATTACHMENT0 + i, tex, i);
                }

                if (mCreateDepthTexture) {
                    attachFramebufferTexture(GL_DEPTH_ATTACHMENT, mDepthTexture, 0);
                } else {
                    glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbuffer);
                    GL_CHECK();
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderbuffer);
                    GL_CHECK();
                }

                GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
                assert(textureCount() <= array_size(DrawBuffers));
                glDrawBuffers(textureCount(), DrawBuffers);
                GL_CHECK();

                if (GLenum check = glCheckFramebufferStatus(GL_FRAMEBUFFER); check != GL_FRAMEBUFFER_COMPLETE) {
                    if (check == 0) {
                        GL_CHECK();
                    } else {
                        LOG_ERROR("Incomplete Framebuffer Status: " << check);
                        glDump();
                    }
                    std::terminate();
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                GL_CHECK();
            }
        }

        for (size_t i = 0; i < mFramebufferCount; ++i) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, it->second[i]);
            GL_CHECK();
            glBindFramebuffer(GL_READ_FRAMEBUFFER, inputTex->mFramebuffers[inputTex->mFlipFlopIndices][i]);
            GL_CHECK();

            for (size_t j = 0; j < count; ++j) {
                GLenum attachment = GL_COLOR_ATTACHMENT0 + j;
                glReadBuffer(attachment);
                GL_CHECK();
#if !OPENGL_ES
                glDrawBuffer(attachment);
#else
                glDrawBuffers(1, &attachment);
#endif
                GL_CHECK();
                glBlitFramebuffer(0, 0, inputTex->mSize.x, inputTex->mSize.y, 0, 0, mSize.x, mSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
                GL_CHECK();
            }
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        GL_CHECK();
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        GL_CHECK();
    }

    Matrix4 OpenGLRenderTexture::getClipSpaceMatrix() const
    {
        return Matrix4 { 1, 0, 0, 0,
            0, -1, 0, 0,
            0, 0, 2, -1,
            0, 0, 0, 1 };
    }

    size_t OpenGLRenderTexture::getFramebufferCount(bool *emulateCube) const
    {
        size_t count = 1;

#if OPENGL_ES < 32
#    if !OPENGL_ES
        if (!glFramebufferTexture)
#    endif
        {
            if (mType == TextureType_Cube) {
                assert(count == 1);
                count = 6;
                if (emulateCube)
                    *emulateCube = true;
            }
        }
#endif
        return count;
    }

    void OpenGLRenderTexture::flipTextures(size_t startIndex, size_t count)
    {
        for (size_t i = 0; i < count; ++i) {
            mFlipFlopIndices[startIndex + i] = mFlipFlopIndices[startIndex + i] ^ 1;
        }
    }

    Vector2i OpenGLRenderTexture::size() const
    {
        return mSize;
    }
}
}
