#ifndef TINY_OPENGL_H
#define TINY_OPENGL_H

#include "tiny_log.h"
#include <glad/glad.h>

void GLAPIENTRY
OglDebugMessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam );

#ifdef TINY_DEBUG
#define GLCall(_CALL)  \
    do { \
        _CALL; GLenum gl_err = glGetError(); \
        if (gl_err != 0) { \
            LOG_FATAL("GL error 0x%x returned from '%s' %s:%i.", gl_err, #_CALL, __FILE__, __LINE__); \
            DEBUG_BREAK; \
            TINY_ASSERT(false); \
        } \
    } while (0)
#else
#define GLCall(_CALL)  _CALL   // Call without error check
#endif


#endif