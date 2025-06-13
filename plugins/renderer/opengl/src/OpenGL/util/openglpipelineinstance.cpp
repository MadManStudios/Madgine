#include "../opengllib.h"

#include "openglpipelineinstance.h"
#include "openglshader.h"

#include "Meta/math/matrix4.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Generic/bytebuffer.h"

#include "openglvertexarray.h"

#include "../openglmeshdata.h"

#include "../openglrendercontext.h"
#include "../openglrendertarget.h"

namespace Engine {
namespace Render {

    size_t uniformAlignment()
    {
        static size_t alignment = []() {
            GLint alignment;
            glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
            GL_CHECK();
            return alignment;
        }();
        return alignment;
    }

    static constexpr GLenum sModes[] {
        GL_POINTS,
        GL_LINES,
        GL_TRIANGLES
    };

    OpenGLPipelineInstance::OpenGLPipelineInstance(const PipelineConfiguration &config, GLuint pipeline)
        : PipelineInstance(config)
        , mHandle(pipeline)
        , mConstantBufferSizes(config.bufferSizes)
        , mDepthChecking(config.depthChecking)
    {
    }

    bool OpenGLPipelineInstance::bind(VertexFormat format, size_t offset) const
    {
        glUseProgram(mHandle);
        GL_CHECK();

        if (mDepthChecking)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

#if !OPENGL_ES || OPENGL_ES >= 31
        OpenGLRenderContext::getSingleton().bindFormat(format, offset);
#else
        mBufferOffset = offset;
        mFormat = format;
#endif

        return true;
    }

    WritableByteBuffer OpenGLPipelineInstance::mapParameters(size_t index)
    {
        size_t size = mConstantBufferSizes[index];

        Block block = OpenGLRenderContext::getSingleton().mTempAllocator.allocate(size, uniformAlignment());
        auto [buffer, offset] = OpenGLRenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);
#if !OPENGL_ES
        glBindBufferRange(GL_UNIFORM_BUFFER, index, buffer, offset, block.mSize);
        GL_CHECK();

        return { block.mAddress, block.mSize };
#else
        glBindBufferRange(GL_UNIFORM_BUFFER, index, buffer, offset, size);
        GL_CHECK();

        GLint location = glGetUniformBlockIndex(mHandle, ("buffer" + std::to_string(index)).c_str());
        GL_CHECK();

        if (location != -1) {
            glUniformBlockBinding(mHandle, location, index);
            GL_CHECK();
        }

        struct Deleter {
            void operator()(std::byte *data) const
            {
                glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
                GL_CHECK();

                glBufferSubData(GL_UNIFORM_BUFFER, mOffset, mSize, data);
                GL_CHECK();

                delete[] data;
            }

            GLuint mBuffer;
            size_t mOffset;
            size_t mSize;
        };

        return { std::unique_ptr<std::byte[], Deleter> { new std::byte[size], { buffer, offset, size } }, size };

#endif
    }

    void OpenGLPipelineInstance::render(RenderTarget *target) const
    {
#if OPENGL_ES
#    if OPENGL_ES < 31
        OpenGLRenderContext::getSingleton().bindFormat(mFormat, mBufferOffset);
#    elif OPENGL_ES < 32
        glBindVertexBuffer(0, mBuffer, mBufferOffset, mStride);
        GL_CHECK();
#    endif

        int location = glGetUniformLocation(mHandle, "SRGB_FRAMEBUFFER");
        GL_CHECK();
        if (location != -1) {
            glUniform1i(location, static_cast<OpenGLRenderTarget *>(target)->mIsSRGBTarget);
            GL_CHECK();
        }
#endif        

        if (mHasIndices) {
            glDrawElements(mMode, mElementCount, GL_UNSIGNED_INT, reinterpret_cast<const void *>(mIndexOffset));
        } else
            glDrawArrays(mMode, 0, mElementCount);
        GL_CHECK();

        static_cast<OpenGLRenderTarget *>(target)->context()->unbindFormat();

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        mHasIndices = false;
    }

    void OpenGLPipelineInstance::renderRange(RenderTarget *target, size_t elementCount, size_t vertexOffset, IndexType<size_t> indexOffset) const
    {
        assert(elementCount <= mElementCount);

#if OPENGL_ES
#    if OPENGL_ES < 31
        OpenGLRenderContext::getSingleton().bindFormat(mFormat, mBufferOffset + vertexOffset * mStride);
        vertexOffset = 0;
#    elif OPENGL_ES < 32
        glBindVertexBuffer(0, mBuffer, mBufferOffset + vertexOffset * mStride, mStride);
        vertexOffset = 0;
        GL_CHECK();
#    endif

        int location = glGetUniformLocation(mHandle, "SRGB_FRAMEBUFFER");
        GL_CHECK();
        if (location != -1) {
            glUniform1i(location, static_cast<OpenGLRenderTarget *>(target)->mIsSRGBTarget);
            GL_CHECK();
        }
#endif

        if (mHasIndices) {
            assert(indexOffset);
#if !OPENGL_ES || OPENGL_ES >= 32
            glDrawElementsBaseVertex(mMode, elementCount, GL_UNSIGNED_INT, reinterpret_cast<const void *>(mIndexOffset + indexOffset * sizeof(uint32_t)), vertexOffset);
#else
            assert(vertexOffset == 0);
            glDrawElements(mMode, elementCount, GL_UNSIGNED_INT, reinterpret_cast<const void *>(mIndexOffset + indexOffset * sizeof(uint32_t)));
#endif
        } else
            glDrawArrays(mMode, vertexOffset, elementCount);
        GL_CHECK();
    }

    void OpenGLPipelineInstance::renderInstanced(RenderTarget *target, size_t count) const
    {
#if OPENGL_ES
#    if OPENGL_ES < 31
        OpenGLRenderContext::getSingleton().bindFormat(mFormat, mBufferOffset);
#    elif OPENGL_ES < 32
        glBindVertexBuffer(0, mBuffer, mBufferOffset, mStride);
        GL_CHECK();
#    endif

        int location = glGetUniformLocation(mHandle, "SRGB_FRAMEBUFFER");
        GL_CHECK();
        if (location != -1) {
            glUniform1i(location, static_cast<OpenGLRenderTarget *>(target)->mIsSRGBTarget);
            GL_CHECK();
        }
#endif

        if (mHasIndices) {
            glDrawElementsInstanced(mMode, mElementCount, GL_UNSIGNED_INT, reinterpret_cast<const void *>(mIndexOffset), count);
        } else
            glDrawArraysInstanced(mMode, 0, mElementCount, count);
        GL_CHECK();

        static_cast<OpenGLRenderTarget *>(target)->context()->unbindFormat();

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        mHasIndices = false;
    }

    WritableByteBuffer OpenGLPipelineInstance::mapTempBuffer(size_t space, size_t size) const
    {
        Block block = OpenGLRenderContext::getSingleton().mTempAllocator.allocate(size, uniformAlignment());
        auto [buffer, offset] = OpenGLRenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);

#if !OPENGL_ES
        glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4 + (space - 1), buffer, offset, block.mSize);
        GL_CHECK();

        return { block.mAddress, block.mSize };
#else

        size_t index = 4 + (space - 1);

        glBindBufferRange(GL_UNIFORM_BUFFER, index, buffer, offset, size);
        GL_CHECK();

        GLint location = glGetUniformBlockIndex(mHandle, ("buffer" + std::to_string(index)).c_str());
        GL_CHECK();

        glUniformBlockBinding(mHandle, location, index);
        GL_CHECK();

        struct Deleter {
            void operator()(std::byte *data) const
            {
                glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
                GL_CHECK();

                glBufferSubData(GL_UNIFORM_BUFFER, mOffset, mSize, data);
                GL_CHECK();

                delete[] data;
            }

            GLuint mBuffer;
            size_t mOffset;
            size_t mSize;
        };

        return { std::unique_ptr<std::byte[], Deleter> { new std::byte[size], { buffer, offset, size } }, size };

#endif
    }

    void OpenGLPipelineInstance::bindMesh(RenderTarget *target, const GPUMeshData *m) const
    {
        const OpenGLMeshData *mesh = static_cast<const OpenGLMeshData *>(m);

        if (!bind(mesh->mFormat))
            return;

#if !OPENGL_ES || OPENGL_ES >= 32
        mesh->mVertices.bindVertex(mesh->mVertexSize);
#else
        glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertices.handle());

        mBuffer = mesh->mVertices.handle();
        mBufferOffset = 0;
        mStride = mesh->mFormat.stride();
#endif

        mMode = sModes[mesh->mGroupSize - 1];

        if (mesh->mIndices) {
            mesh->mIndices.bind();
            mHasIndices = true;
        } else {
            mHasIndices = false;
        }

        mElementCount = mesh->mElementCount;

        mIndexOffset = 0;
    }

    WritableByteBuffer OpenGLPipelineInstance::mapVertices(RenderTarget *target, VertexFormat format, size_t count) const
    {
        size_t size = format.stride() * count;

        Block block = OpenGLRenderContext::getSingleton().mTempAllocator.allocate(size);
        auto [buffer, offset] = OpenGLRenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);

        if (!bind(format, offset))
            return {};

        glBindBuffer(GL_ARRAY_BUFFER, buffer);

#if !OPENGL_ES || OPENGL_ES >= 32
        glBindVertexBuffer(0, buffer, offset, format.stride());
        GL_CHECK();
#else
        mBuffer = buffer;
        mBufferOffset = offset;
        mStride = format.stride();
#endif

        mElementCount = count;

#if !OPENGL_ES
        return { block.mAddress, block.mSize };
#else
        struct Deleter {
            void operator()(std::byte *data) const
            {
                glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
                GL_CHECK();

                glBufferSubData(GL_UNIFORM_BUFFER, mOffset, mSize, data);
                GL_CHECK();

                delete[] data;
            }

            GLuint mBuffer;
            size_t mOffset;
            size_t mSize;
        };

        return { std::unique_ptr<std::byte[], Deleter> { new std::byte[size], { buffer, offset, size } }, size };
#endif
    }

    ByteBufferImpl<uint32_t> OpenGLPipelineInstance::mapIndices(RenderTarget *target, size_t count) const
    {
        size_t size = sizeof(uint32_t) * count;

#if !EMSCRIPTEN
        Block block = OpenGLRenderContext::getSingleton().mTempAllocator.allocate(size);
        auto [buffer, offset] = OpenGLRenderContext::getSingleton().mTempMemoryHeap.resolve(block.mAddress);
#else
        Block block = OpenGLRenderContext::getSingleton().mTempIndexAllocator.allocate(size);
        auto [buffer, offset] = OpenGLRenderContext::getSingleton().mTempIndexMemoryHeap.resolve(block.mAddress);
#endif

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        GL_CHECK();

        mIndexOffset = offset;

        mElementCount = count;
        mHasIndices = true;

#if !OPENGL_ES
        return { static_cast<uint32_t *>(block.mAddress), block.mSize };
#else
        struct Deleter {
            void operator()(uint32_t *data) const
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
                GL_CHECK();

                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, mOffset, mSize, data);
                GL_CHECK();

                delete[] data;
            }

            GLuint mBuffer;
            size_t mOffset;
            size_t mSize;
        };

        return { std::unique_ptr<uint32_t[], Deleter> { new uint32_t[count], { buffer, offset, size } }, size };
#endif
    }

    void OpenGLPipelineInstance::setGroupSize(size_t groupSize) const
    {
        mMode = sModes[groupSize - 1];
    }

    void OpenGLPipelineInstance::bindResources(RenderTarget *target, size_t space, ResourceBlock block) const
    {
        if (block) {
            OpenGLResourceBlock<> *textures = block;
            for (size_t i = 0; i < textures->mSize; ++i) {
                size_t index = 4 * (space - 1) + i;
                glActiveTexture(GL_TEXTURE0 + index);
                glBindTexture(textures->mResources[i].mTarget, textures->mResources[i].mHandle);
                GL_CHECK();

#if OPENGL_ES && OPENGL_ES < 31
                GLint location = glGetUniformLocation(mHandle, ("texture" + std::to_string(index)).c_str());
                GL_CHECK();
                glUseProgram(mHandle);
                glUniform1i(location, index);
                GL_CHECK();
#endif
            }
        }
    }

    OpenGLPipelineInstanceHandle::OpenGLPipelineInstanceHandle(const PipelineConfiguration &config, OpenGLPipelineLoader::Handle pipeline)
        : OpenGLPipelineInstance(config, pipeline->handle())
        , mPipeline(std::move(pipeline))
    {
    }

    OpenGLPipelineInstancePtr::OpenGLPipelineInstancePtr(const PipelineConfiguration &config, OpenGLPipelineLoader::Ptr pipeline)
        : OpenGLPipelineInstance(config, pipeline->handle())
        , mPipeline(std::move(pipeline))
    {
    }
}
}