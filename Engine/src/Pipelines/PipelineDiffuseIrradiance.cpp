#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Helpers/PrismaRender.h"

Prisma::PipelineDiffuseIrradiance::PipelineDiffuseIrradiance()
{
    m_shader = std::make_shared<Shader>("../../../Engine/Shaders/IrradiancePipeline/vertex.glsl", "../../../Engine/Shaders/IrradiancePipeline/fragment.glsl");
}

void Prisma::PipelineDiffuseIrradiance::texture(Prisma::Texture texture)
{
    // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
// --------------------------------------------------------------------------------
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, Prisma::PrismaRender::getInstance().data().fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, Prisma::PrismaRender::getInstance().data().rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
    // -----------------------------------------------------------------------------
    m_shader->use();
    m_shader->setInt64(m_shader->getUniformPosition("environmentMap"), texture.id());
    m_shader->setMat4(m_shader->getUniformPosition("projection"), Prisma::PrismaRender::getInstance().data().captureProjection);

    GLint viewport[4];

    glGetIntegerv(GL_VIEWPORT, viewport);

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, Prisma::PrismaRender::getInstance().data().fbo);
    for (unsigned int i = 0; i < 6; ++i)
    {
        m_shader->setMat4(m_shader->getUniformPosition("view"), Prisma::PrismaRender::getInstance().data().captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Prisma::PrismaRender::getInstance().renderCube();
    }
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]); // don't forget to configure the viewport to the capture dimensions.

    m_id = glGetTextureHandleARB(irradianceMap);
    glMakeTextureHandleResidentARB(m_id);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint64_t Prisma::PipelineDiffuseIrradiance::id()
{
    return m_id;
}
