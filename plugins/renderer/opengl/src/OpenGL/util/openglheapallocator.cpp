#include "../opengllib.h"

#include "openglheapallocator.h"

#include "Generic/align.h"

#include "Madgine/render/ptr.h"

namespace Engine {
namespace Render {

    OpenGLHeapAllocator::OpenGLHeapAllocator()
    {
    }

    Block OpenGLHeapAllocator::allocate(size_t size, size_t alignment)
    {
        GLuint buffer;

        glGenBuffers(1, &buffer);
        GL_CHECK();

        glBindBuffer(GL_COPY_WRITE_BUFFER, buffer);
        GL_CHECK();

        glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
        GL_CHECK();

        return { reinterpret_cast<void *>(static_cast<uintptr_t>(buffer) << 24), size };
    }

    void OpenGLHeapAllocator::deallocate(Block block)
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(block.mAddress);
        uint32_t buffer = ptr >> 24;
        uint32_t offset = ptr & ((1 << 24) - 1);
        assert(offset == 0 && buffer != 0);

        glDeleteBuffers(1, &buffer);
        GL_CHECK();
    }

    std::pair<GLuint, size_t> OpenGLHeapAllocator::resolve(void *ptr)
    {
        uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
        uint32_t buffer = address >> 24;
        uint32_t offset = address & ((1 << 24) - 1);
        return { buffer, offset };
    }

    OpenGLMappedHeapAllocator::OpenGLMappedHeapAllocator(
#if OPENGL_ES
        GLenum target
#endif
        )
#if OPENGL_ES
        : mTarget(target)
#endif
    {
    }

    Block OpenGLMappedHeapAllocator::allocate(size_t size, size_t alignment)
    {
#if !OPENGL_ES
        GLuint handle;

        glCreateBuffers(1, &handle);
        GL_CHECK();

        glNamedBufferStorage(handle, size, nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
        GL_CHECK();

        void *ptr = glMapNamedBufferRange(handle, 0, size, GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
        GL_CHECK();

        mPages.push_back({ handle,
            ptr,
            size });

        return { ptr, size };

#else
        GLuint handle;
        glGenBuffers(1, &handle);
        GL_CHECK();

        glBindBuffer(mTarget, handle);
        GL_CHECK();

        glBufferData(mTarget, size, nullptr, GL_DYNAMIC_DRAW);
        GL_CHECK();

        glBindBuffer(mTarget, 0);
        GL_CHECK();

        return { reinterpret_cast<void *>(static_cast<uintptr_t>(handle) << 24), size };
#endif
    }

    void OpenGLMappedHeapAllocator::deallocate(Block block)
    {
#if !OPENGL_ES
        auto it = std::ranges::find(mPages, block.mAddress, &Page::mMappedAddress);
        assert(it != mPages.end());

        glDeleteBuffers(1, &it->mBuffer);

        mPages.erase(it);
#else
        GLuint handle = reinterpret_cast<uintptr_t>(block.mAddress) >> 24;

        glDeleteBuffers(1, &handle);
#endif
    }

    std::pair<GLuint, size_t> OpenGLMappedHeapAllocator::resolve(void *ptr)
    {
#if !OPENGL_ES
        auto it = std::ranges::find_if(mPages, [=](const Page &page) {
            return page.mMappedAddress <= ptr && ptr < static_cast<std::byte *>(page.mMappedAddress) + page.mSize;
        });
        assert(it != mPages.end());
        return { it->mBuffer, static_cast<std::byte *>(ptr) - static_cast<std::byte *>(it->mMappedAddress) };
#else
        GLuint handle = reinterpret_cast<uintptr_t>(ptr) >> 24;
        uint32_t offset = reinterpret_cast<uintptr_t>(ptr) & ((1 << 24) - 1);

        return { handle, offset };
#endif
    }

}
}