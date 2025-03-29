#pragma once

#include "Meta/math/vector2i.h"

#include "rendertextureconfig.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#include "Modules/threading/taskfuture.h"

#include "Madgine/render/buffer.h"

#include "Madgine/render/resourceblock.h"

#include "Generic/bytebuffer.h"

#include "Madgine/render/future.h"

namespace Engine {
namespace Render {

    struct MADGINE_RENDER_EXPORT RenderContext {
        RenderContext(Threading::TaskQueue *queue);
        virtual ~RenderContext();

        virtual std::unique_ptr<RenderTarget> createRenderWindow(Window::OSWindow *w, size_t samples = 1) = 0;
        virtual std::unique_ptr<RenderTarget> createRenderTexture(const Vector2i &size = { 1, 1 }, const RenderTextureConfig &config = {}) = 0;

        virtual Threading::Task<void> unloadAllResources();

        void addRenderTarget(RenderTarget *target);
        void removeRenderTarget(RenderTarget *target);
        std::vector<const RenderTarget *> renderTargets() const;

        Threading::Task<void> render();

        virtual bool beginFrame();
        virtual void endFrame();

        static RenderContext &getSingleton();

        size_t frame() const;

        static Threading::TaskQueue *renderQueue();

        virtual bool supportsMultisampling() const = 0;

        template <typename T>
        requires (!std::is_array_v<T>)
        GPUBuffer<T> allocateBuffer()
        {
            return static_cast<GPUBuffer<T>>(allocateBufferImpl(sizeof(T)));
        }

        template <typename T>
        requires std::is_unbounded_array_v<T>
            GPUBuffer<T> allocateBuffer(size_t elementCount)
        {
            return static_cast<GPUBuffer<T>>(allocateBufferImpl(sizeof(std::remove_extent_t<T>) * elementCount));
        }

        template <typename T>
        void deallocateBuffer(GPUBuffer<T> buffer)
        {
            deallocateBufferImpl(std::move(buffer));
        }

        template <typename T>
        auto mapBuffer(GPUBuffer<T> &buffer)
        {
            return mapBufferImpl(buffer).template cast<T>();
        }

        virtual UniqueResourceBlock createResourceBlock(std::vector<const Texture*> textures) = 0;
        virtual void destroyResourceBlock(UniqueResourceBlock &block) = 0;

    protected:
        virtual GPUBuffer<void> allocateBufferImpl(size_t size) { throw 0; };
        virtual void deallocateBufferImpl(GPUBuffer<void> buffer) { throw 0; };
        virtual WritableByteBuffer mapBufferImpl(GPUBuffer<void> &buffer) { throw 0; };

    protected:
        void checkThread();

        static bool isRenderThread();

    protected:
        std::vector<RenderTarget *> mRenderTargets;

        Threading::TaskQueue *mRenderQueue = nullptr;
        std::thread::id mRenderThread;

        size_t mFrame = 1;
    };

}
}

REGISTER_TYPE(Engine::Render::RenderContext)