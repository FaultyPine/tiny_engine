#ifndef TINY_OPENGL_H
#define TINY_OPENGL_H

#include "tiny_log.h"
//#define GLAD_GLAPI_EXPORT
#include <glad/glad.h>

void GLAPIENTRY
OglDebugMessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam );

#define GLCall(_CALL)  _CALL   // Call without error check


void OGLDrawDefault(u32 VAO, u32 indicesSize, u32 verticesSize);
void OGLDrawInstanced(u32 VAO, u32 indicesSize, u32 verticesSize, u32 numInstances);
void ConfigureVertexAttrib(u32 attributeLoc, u32 numComponentsInAttribute, u32 oglType, bool shouldNormalize, u32 stride, void* offset);


#endif