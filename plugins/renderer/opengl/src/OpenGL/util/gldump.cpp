#include "../opengllib.h"

#include "openglvertexarray.h"

using namespace Engine::Render;

void glDump()
{

    Engine::Log::LogDummy cout { Engine::Log::MessageType::INFO_TYPE };

    cout << "GL-State: ---------- \n";

    GLenum glenum;
    GLint glint;

    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &glint);
    cout << "GL_ARRAY_BUFFER: " << glint << "\n";

    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &glint);
    cout << "GL_ELEMENT_ARRAY_BUFFER: " << glint << "\n";

    cout << "GL-State End ---------";
}