#pragma once

#include "openglrendertarget.h"
#include "util/opengltexture.h"

namespace Engine {
namespace Render {

    struct MADGINE_OPENGL_EXPORT OpenGLRenderTexture : OpenGLRenderTarget {

        OpenGLRenderTexture(OpenGLRenderContext *context, const Vector2i &size, const RenderTextureConfig &config);
        ~OpenGLRenderTexture();

        bool resizeImpl(const Vector2i &size) override;
        Vector2i size() const override;

        virtual void beginIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;
        virtual void endIteration(size_t targetIndex, size_t targetCount, size_t targetSubresourceIndex) const override;

        virtual void beginFrame() override;
        virtual RenderFuture endFrame() override;

        virtual const OpenGLTexture *texture(size_t index) const override;
        virtual size_t textureCount() const override;
        virtual const OpenGLTexture *depthTexture() const override;

        void blit(RenderTarget *input) const;

        virtual Matrix4 getClipSpaceMatrix() const override;

    protected:
        size_t getFramebufferCount(bool *emulateCube = nullptr) const;

        void flipTextures(size_t startIndex, size_t count) override;

    private:
        mutable std::map<BitArray<4>, std::array<GLuint, 6>> mFramebuffers;
        GLuint mDepthRenderbuffer = 0;

        OpenGLTexture mDepthTexture;
        bool mCreateDepthTexture;

        size_t mSamples;

        std::vector<OpenGLTexture> mTextures;

        Vector2i mSize;

        TextureType mType;

        size_t mFramebufferCount;
    };

}
}