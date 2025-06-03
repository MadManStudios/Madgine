#pragma once

#include "Madgine/render/vertexformat.h"

#include "Generic/bytebuffer.h"

namespace Engine {
namespace Render {

    struct GPUMeshData;
    struct RenderTarget;
    struct ResourceBlock;

    struct PipelineConfiguration {
        std::string_view vs;
        std::string_view ps;
        std::vector<size_t> bufferSizes;
        std::vector<std::string> postProcessing;
        bool depthChecking = true;
    };

    struct MADGINE_RENDER_EXPORT PipelineInstance {
        PipelineInstance(const PipelineConfiguration &config)
        {
        }
        PipelineInstance(const PipelineInstance &) = delete;
        virtual ~PipelineInstance() = default;

        PipelineInstance &operator=(const PipelineInstance &) = delete;

        virtual WritableByteBuffer mapParameters(size_t index) = 0;
        template <typename T>
        ByteBufferImpl<T> mapParameters(size_t index)
        {
            return mapParameters(index).cast<T>();
        }

        virtual WritableByteBuffer mapTempBuffer(size_t space, size_t size) const = 0;
        template <typename T>
        requires(!std::is_array_v<T>)
            ByteBufferImpl<T> mapTempBuffer(size_t space)
        {
            return mapTempBuffer(space, sizeof(T)).cast<T>();
        }
        template <typename T>
        requires std::is_unbounded_array_v<T>
            ByteBufferImpl<T> mapTempBuffer(size_t space, size_t count)
        {
            return mapTempBuffer(space, sizeof(std::remove_extent_t<T>) * count).cast<T>();
        }

        virtual void bindMesh(RenderTarget *target, const GPUMeshData *mesh) const = 0;
        virtual ByteBufferImpl<uint32_t> mapIndices(RenderTarget *target, size_t count) const = 0;
        virtual WritableByteBuffer mapVertices(RenderTarget *target, VertexFormat format, size_t count) const = 0;
        template <typename T>
        requires std::is_unbounded_array_v<T>
            ByteBufferImpl<T> mapVertices(RenderTarget *target, size_t count)
        {
            return mapVertices(target, type_holder<std::remove_extent_t<T>>, count).template cast<T>();
        }       
        virtual void setGroupSize(size_t groupSize) const = 0;

        virtual void render(RenderTarget *target) const = 0;
        virtual void renderRange(RenderTarget *target, size_t elementCount, size_t vertexOffset, IndexType<size_t> indexOffset = {}) const = 0;
        virtual void renderInstanced(RenderTarget *target, size_t count) const = 0;
        template <typename T>
        void renderInstanced(RenderTarget *target, T &&instanceData) const
        {
            ByteBufferImpl buffer { std::forward<T>(instanceData) };
            renderInstanced(target, buffer.elementCount(), std::move(buffer).template cast<const void>());
        }

        void renderQuad(RenderTarget *target) const;

        virtual void bindResources(RenderTarget *target, size_t space, ResourceBlock block) const = 0;
    };

}
}