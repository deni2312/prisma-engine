#include "../../include/Containers/FBO.h"
#include "../../include/Helpers/GarbageCollector.h"
#include <iostream>

Prisma::FBO::FBO(FBOData fboData)
    : m_fboData{ fboData }, m_vao{0} {

    glGenFramebuffers(1, &m_framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);

    // Create a color attachment texture
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, m_fboData.internalFormat, fboData.width, fboData.height, 0, GL_RGBA, m_fboData.internalType, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    if (m_fboData.enableDepth) {
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_fboData.width, m_fboData.height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
        Prisma::GarbageCollector::getInstance().add({Prisma::GarbageCollector::GarbageType::RBO,rbo});
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    m_id = glGetTextureHandleARB(textureID);
    glMakeTextureHandleResidentARB(m_id);
    Prisma::GarbageCollector::getInstance().add({Prisma::GarbageCollector::GarbageType::FBO,m_framebufferID});
    Prisma::GarbageCollector::getInstance().addTexture({ textureID,m_id});

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Prisma::FBO::~FBO() {

}

void Prisma::FBO::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
}

void Prisma::FBO::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint64_t Prisma::FBO::texture() const {
    return m_id;
}

unsigned int Prisma::FBO::frameBufferID()
{
    return m_framebufferID;
}
