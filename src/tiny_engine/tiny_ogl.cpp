#include "tiny_ogl.h"


static bool GLLogCall(const char* func, const char* file, int line) {
    while (GLenum error = glGetError()) {
        LOG_ERROR("OpenGL error: (%i): %s:%i\n", error, file, line);
        return false;
    }
    return true;
}