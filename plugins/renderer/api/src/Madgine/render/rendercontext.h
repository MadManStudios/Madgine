#pragma once

#include "Generic/bytebuffer.h"

#include "Meta/math/vector2i.h"

#include "Modules/threading/taskfuture.h"
#include "Modules/uniquecomponent/uniquecomponent.h"

#include "Madgine/render/ptr.h"
#include "Madgine/render/future.h"
#include "Madgine/render/resourceblock.h"

#include "rendertextureconfig.h"

namespace Engine {
namespace Render {

    enum class UsageHint {
        USAGE_DEFAULT,
        USAGE_INDEX
    };

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
            requires(!std::is_array_v<T>)
        GPUPtr<T> allocateBuffer(UsageHint hint = UsageHint::USAGE_DEFAULT)
        {
            return static_cast<GPUPtr<T>>(allocateBufferImpl(sizeof(T), hint));
        }

        template <typename T>
            requires std::is_unbounded_array_v<T>
        GPUPtr<T> allocateBuffer(size_t elementCount, UsageHint hint = UsageHint::USAGE_DEFAULT)
        {
            return static_cast<GPUPtr<T>>(allocateBufferImpl(sizeof(std::remove_extent_t<T>), elementCount, hint));
        }

        template <typename T>
        auto mapBuffer(GPUPtr<T> &buffer)
        {
            return mapBufferImpl(buffer).template cast<T>();
        }

        virtual TexturePtr createTexture(TextureType type, TextureFormat format, Vector2i size = { 0, 0 }, const ByteBuffer &data = {}) = 0;
        
        virtual void setTextureSubData(const TexturePtr &tex, Vector2i offset, Vector2i size, const ByteBuffer &data) = 0;

        virtual UniqueResourceBlock createResourceBlock(std::vector<std::variant<ConstTexturePtr, GPUPtr<void>, GPUPtr<Void[]>>> data) = 0;
        virtual void destroyResourceBlock(UniqueResourceBlock &block) = 0;

    protected:
        virtual GPUPtr<void> allocateBufferImpl(size_t size, UsageHint hint = UsageHint::USAGE_DEFAULT) { throw 0; };
        virtual GPUPtr<Void[]> allocateBufferImpl(size_t elementSize, size_t count, UsageHint hint = UsageHint::USAGE_DEFAULT) { throw 0; };
        virtual WritableByteBuffer mapBufferImpl(const GPUPtr<void> &buffer) { throw 0; };
        virtual WritableByteBuffer mapBufferImpl(const GPUPtr<Void[]> &buffer) { throw 0; };

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