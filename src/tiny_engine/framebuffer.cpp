#include "framebuffer.h"
#include "camera.h"


// by default this framebuffer is just for the whole screen
FullscreenFrameBuffer::FullscreenFrameBuffer(glm::vec2 framebufferSize) {
    this->size = framebufferSize;
    // generate and bind a framebuffer object
    glGenFramebuffers(1, &framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);    

    // generate a texture so that writes to this framebuffer go into the texture
    // this is useful because we can then use this texture in our shaders
    glGenTextures(1, &textureColorBufferID);
    glBindTexture(GL_TEXTURE_2D, textureColorBufferID);
    // allocate mem for the texture, but don't fill it (hence the NULL at the end)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // should this go after the glFramebufferTexture2D?

    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBufferID, 0); 

    // to make sure opengl can do depth testing (or stencil testing), gotta add a depth/stencil attachment
    // since we're only actually (in this senario) sampling the color and not the other buffers,
    // using a renderbuffer object for depth/stencil is great for this
    glGenRenderbuffers(1, &renderBufferObjectID);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObjectID); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0); // should this go after the glFramebufferRenderbuffer?

    // attach the renderbuffer object to the depth and stencil attachment of the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObjectID);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  


    fullscreenSprite = Sprite(Texture(textureColorBufferID));

}