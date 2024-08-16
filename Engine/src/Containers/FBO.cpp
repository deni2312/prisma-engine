#include "../../include/Containers/FBO.h"
#include "../../include/Helpers/GarbageCollector.h"
#include <iostream>
#include "../../../GUI/include/TextureInfo.h"

Prisma::FBO::FBO(FBOData fboData)
    : m_fboData{ fboData }, m_vao{0} {


    glGenFramebuffers(1, &m_framebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);

    // Create a color attachment texture
    unsigned int textureID;
    glGenTextures(1, &textureID);

    if (m_fboData.enableMultisample) {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, m_fboData.internalFormat, m_fboData.width, m_fboData.height, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureID, 0);
        // create a (also multisampled) renderbuffer object for depth and stencil attachments
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, m_fboData.width, m_fboData.height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    }
    else {

        if (m_fboData.enableColor) {
            glBindTexture(GL_TEXTURE_2D, textureID);

            glTexImage2D(GL_TEXTURE_2D, 0, m_fboData.internalFormat, fboData.width, fboData.height, 0, GL_RGBA, m_fboData.internalType, NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_fboData.filtering);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_fboData.filtering);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_fboData.border);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_fboData.border);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
        }

        if (m_fboData.enableDepth) {
            if (m_fboData.rbo) {
                unsigned int rbo;
                glGenRenderbuffers(1, &rbo);
                glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_fboData.width, m_fboData.height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
                Prisma::GarbageCollector::getInstance().add({ Prisma::GarbageCollector::GarbageType::RBO,rbo });
            }
            else {
                unsigned int depthTexture;

                // Generate and configure the depth texture
                glGenTextures(1, &depthTexture);
                glBindTexture(GL_TEXTURE_2D, depthTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_fboData.width, m_fboData.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

                m_depthId = glGetTextureHandleARB(depthTexture);
                glMakeTextureHandleResidentARB(m_depthId);
            }
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }

    m_id = glGetTextureHandleARB(textureID);
    glMakeTextureHandleResidentARB(m_id);
    Prisma::GarbageCollector::getInstance().add({Prisma::GarbageCollector::GarbageType::FBO,m_framebufferID});
    Prisma::GarbageCollector::getInstance().addTexture({ textureID,m_id});
    Prisma::TextureInfo::getInstance().add({ textureID, m_fboData.name});

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

uint64_t Prisma::FBO::depth() const
{
    return m_depthId;
}

unsigned int Prisma::FBO::frameBufferID()
{
    return m_framebufferID;
}
