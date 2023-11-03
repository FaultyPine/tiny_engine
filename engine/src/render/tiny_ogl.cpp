#include "tiny_ogl.h"

#include "tiny_log.h"

#include <glad/glad.c>

#define OGL_LOG_NOTIFS_ENABLED 0

void GLAPIENTRY
OglDebugMessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
        {
            LOG_ERROR("[GL] %s", message);
            DEBUG_BREAK;
        } break;
        case GL_DEBUG_SEVERITY_MEDIUM:
        {
            LOG_WARN("[GL] %s", message);
        } break;
        case GL_DEBUG_SEVERITY_LOW:
        {
            LOG_INFO("[GL] %s", message);
        } break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        default:
        {
            #if OGL_LOG_NOTIFS_ENABLED
            LOG_TRACE("[GL] %s", message);
            #endif
        } break;
    }
}
