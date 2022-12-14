#include "framebuffer.h"


Framebuffer::Framebuffer(f32 width, f32 height, FramebufferAttachmentType fbtype) {
    this->type = fbtype;
    this->size = glm::vec2(width, height);

    // generate and bind a framebuffer object
    GLCall(glGenFramebuffers(1, &framebufferID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, framebufferID));    

    GLCall(glGenTextures(1, &texture));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture));

    s32 component = type == DEPTH ? GL_DEPTH_COMPONENT : GL_RGB;
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    if (type == DEPTH) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        f32 borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
    }
    else {
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)); 
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)); 
    }
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, component, width, height, 0, component, type == DEPTH ? GL_FLOAT : GL_UNSIGNED_BYTE, NULL));
    if (type == DEPTH) {
        // disable color buffer
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, type, GL_TEXTURE_2D, texture, 0));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));

    // to make sure opengl can do depth testing (or stencil testing), gotta add a depth/stencil attachment
    // since we're only actually (in this senario) sampling the color and not the other buffers,
    // using a renderbuffer object for depth/stencil is great for this
    /*
    GLCall(glGenRenderbuffers(1, &renderBufferObjectID));
    GLCall(glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObjectID)); 
    GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y));  
    GLCall(glBindRenderbuffer(GL_RENDERBUFFER, 0)); // should this go after the glFramebufferRenderbuffer?
    // attach the renderbuffer object to the depth and stencil attachment of the framebuffer
    GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObjectID));
    */
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));  
}