#include "tiny_ogl.h"

#include "tiny_log.h"

#define GLAD_GLAPI_EXPORT
#include <glad/glad.c>

#define OGL_LOG_NOTIFS_ENABLED 0
#define OGL_LOG_LOW_ENABLED 1
#define OGL_LOG_MEDIUM_ENABLED 1
#define OGL_LOG_HIGH_ENABLED 1

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
            #if OGL_LOG_HIGH_ENABLED
            LOG_ERROR("[GL] %s", message);
            #endif
            DEBUG_BREAK;
        } break;
        case GL_DEBUG_SEVERITY_MEDIUM:
        {
            #if OGL_LOG_MEDIUM_ENABLED
            LOG_WARN("[GL] %s", message);
            #endif
        } break;
        case GL_DEBUG_SEVERITY_LOW:
        {
            #if OGL_LOG_LOW_ENABLED
            LOG_INFO("[GL] %s", message);
            #endif
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
