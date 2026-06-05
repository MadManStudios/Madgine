#pragma once

#include "Generic/allocator/bucket.h"
#include "Generic/allocator/bump.h"
#include "Generic/allocator/fixed.h"
#include "Generic/allocator/heap.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/render/rendercontextcollector.h"
#include "Madgine/render/vertexformat.h"

#include "util/openglheapallocator.h"
#include "util/openglvertexarray.h"

namespace Engine {
namespace Render {

    ContextHandle createContext(SurfaceHandle surface, size_t samples, ContextHandle reusedContext, bool setup = true);
    void destroyContext(SurfaceHandle surface, ContextHandle context, bool reusedContext = false);
    void makeCurrent(SurfaceHandle surface, ContextHandle context);
    void swapBuffers(SurfaceHandle surface, ContextHandle context);

    struct MADGINE_OPENGL_EXPORT OpenGLRenderContext : public RenderContextComponent<OpenGLRenderContext> {
        OpenGLRenderContext(Threading::TaskQueue *queue);
        ~OpenGLRenderContext();
        OpenGLRenderContext(const OpenGLRenderContext &) = delete;

        OpenGLRenderContext &operator=(const OpenGLRenderContext &) = delete;

        std::unique_ptr<RenderTarget> createRenderWindow(Platform::Window::OSWindow *w, size_t samples) override;
        std::unique_ptr<RenderTarget> createRenderTexture(const Math::Vector2i &size = { 1, 1 }, const RenderTextureConfig &config = {}) override;

        Threading::Task<void> unloadAllResources() override;

        static OpenGLRenderContext &getSingleton();

        bool supportsMultisampling() const override;

        GPUPtr<void> allocateBufferImpl(size_t size, UsageHint hint = UsageHint::USAGE_DEFAULT) override;
        GPUPtr<Void[]> allocateBufferImpl(size_t elementSize, size_t count, UsageHint hint = UsageHint::USAGE_DEFAULT) override;
        Memory::WritableByteBuffer mapBufferImpl(const GPUPtr<void> &buffer) override;
        Memory::WritableByteBuffer mapBufferImpl(const GPUPtr<Void[]> &buffer) override;

        TexturePtr createTexture(TextureType type, TextureFormat format, Math::Vector2i size, const Memory::ByteBuffer &data) override;

        void setTextureSubData(const TexturePtr &tex, Math::Vector2i offset, Math::Vector2i size, const Memory::ByteBuffer &data) override;

        UniqueResourceBlock createResourceBlock(std::vector<std::variant<ConstTexturePtr, GPUPtr<void>, GPUPtr<Void[]>>> textures) override;
        void destroyResourceBlock(UniqueResourceBlock &block) override;

        void bindFormat(VertexFormat format, size_t offset = 0);
        void unbindFormat();

        OpenGLHeapAllocator mBufferMemoryHeap;
        Memory::LogBucketAllocator<Memory::HeapAllocator<OpenGLHeapAllocator &>, 64, 4096, 4> mBufferAllocator;

        OpenGLMappedHeapAllocator mTempMemoryHeap;
        Memory::BumpAllocator<Memory::FixedAllocator<OpenGLMappedHeapAllocator &>> mTempAllocator;

#if EMSCRIPTEN
        OpenGLMappedHeapAllocator mTempIndexMemoryHeap;
        Memory::BumpAllocator<Memory::FixedAllocator<OpenGLMappedHeapAllocator &>> mTempIndexAllocator;

        OpenGLHeapAllocator mIndexMemoryHeap;
        Memory::LogBucketAllocator<Memory::HeapAllocator<OpenGLHeapAllocator &>, 64, 4096, 4> mIndexAllocator;
#endif

    private:
        std::map<VertexFormat, OpenGLVertexArray> mVAOs;
    };

}
}