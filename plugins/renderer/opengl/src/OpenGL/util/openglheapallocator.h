#pragma once

#include "Generic/allocator/concepts.h"

namespace Engine {
namespace Render {

    struct OpenGLHeapAllocator {

        static constexpr size_t goodSize = 8 * 1024 * 1024; //8MB

        OpenGLHeapAllocator();

        Block allocate(size_t size, size_t alignment = 1);
        void deallocate(Block block);

    private:
    };

    struct OpenGLMappedHeapAllocator {

        static constexpr size_t goodSize = 8 * 1024 * 1024; //8MB

        OpenGLMappedHeapAllocator(
#if OPENGL_ES
            GLenum target
#endif
        );

        Block allocate(size_t size, size_t alignment = 1);
        void deallocate(Block block);

        std::pair<GLuint, size_t> resolve(void *ptr);

    private:
#if !OPENGL_ES
        struct Page {
            GLuint mBuffer;
            void *mMappedAddress;
            size_t mSize;
        };
        std::vector<Page> mPages;
#else
        GLenum mTarget;
#endif
    };

}
}