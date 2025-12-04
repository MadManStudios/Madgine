#pragma once

namespace Engine {
namespace Render {

    struct MADGINE_OPENGL_EXPORT OpenGLVertexArray {

        OpenGLVertexArray() = default;
        OpenGLVertexArray(create_t);
        OpenGLVertexArray(const OpenGLVertexArray &) = delete;
        OpenGLVertexArray(OpenGLVertexArray &&);
        ~OpenGLVertexArray();

        OpenGLVertexArray &operator=(OpenGLVertexArray &&other);

        explicit operator bool() const;

        void reset();

        void bind() const;
        void unbind();

        GLuint handle();

    private:
        GLuint mHandle = 0;
    };

}
}