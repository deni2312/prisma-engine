#include "../../include/Pipelines/PipelineSkybox.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Pipelines/PipelinePrefilter.h"
#include "../../include/Pipelines/PipelineLUT.h"
#include "../../include/Helpers/IBLBuilder.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

std::shared_ptr<Prisma::PipelineSkybox> Prisma::PipelineSkybox::instance = nullptr;

Prisma::PipelineSkybox::PipelineSkybox()
{
    m_shader = std::make_shared<Shader>("../../../Engine/Shaders/SkyboxPipeline/vertex.glsl", "../../../Engine/Shaders/SkyboxPipeline/fragment.glsl");
    m_shaderEquirectangular = std::make_shared<Shader>("../../../Engine/Shaders/EquirectangularPipeline/vertex.glsl", "../../../Engine/Shaders/EquirectangularPipeline/fragment.glsl");
    m_bindlessPos = m_shader->getUniformPosition("skybox");
    m_bindlessPosEquirectangular = m_shaderEquirectangular->getUniformPosition("equirectangularMap");
}

uint64_t Prisma::PipelineSkybox::calculateSkybox()
{

    unsigned int width = m_texture.data().width;
    unsigned int height = m_texture.data().height;

    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------

    m_shaderEquirectangular->use();
    m_shaderEquirectangular->setInt64(m_bindlessPosEquirectangular, m_texture.id());
    m_shaderEquirectangular->setMat4(m_shaderEquirectangular->getUniformPosition("projection"), Prisma::IBLBuilder::getInstance().data().captureProjection);


    auto posView = m_shaderEquirectangular->getUniformPosition("view");
    GLint viewport[4];

    glGetIntegerv(GL_VIEWPORT, viewport);

    glViewport(0, 0, width, height); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, Prisma::IBLBuilder::getInstance().data().fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, Prisma::IBLBuilder::getInstance().data().rbo);
    for (unsigned int i = 0; i < 6; ++i)
    {
        m_shaderEquirectangular->setMat4(posView, Prisma::IBLBuilder::getInstance().data().captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Prisma::IBLBuilder::getInstance().renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]); // don't forget to configure the viewport to the capture dimensions.

    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    uint64_t m_id = glGetTextureHandleARB(envCubemap);
    glMakeTextureHandleResidentARB(m_id);

    return m_id;
}

void Prisma::PipelineSkybox::projection(glm::mat4 projection)
{
	m_projection = projection;
}

void Prisma::PipelineSkybox::render(std::shared_ptr<Camera> camera)
{
    glDepthFunc(GL_LEQUAL);
    m_shader->use();
    auto cameraFinal = glm::mat4(glm::mat3(camera->matrix()));
    MeshHandler::getInstance().ubo()->modifyData(Prisma::MeshHandler::VIEW_OFFSET, sizeof(glm::mat4), glm::value_ptr(cameraFinal));
    m_shader->setInt64(m_bindlessPos, m_skyboxId);
    Prisma::IBLBuilder::getInstance().renderCube();
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

Prisma::PipelineSkybox& Prisma::PipelineSkybox::getInstance()
{
    if (!instance) {
        instance = std::make_shared<PipelineSkybox>();
    }
    return *instance;
}

void Prisma::PipelineSkybox::texture(Prisma::Texture texture,bool equirectangular)
{
    m_texture = texture;
    m_equirectangular = equirectangular;
    m_skyboxId = texture.id();
    if (m_equirectangular) {
        Prisma::IBLBuilder::getInstance().createFbo(texture.data().width, texture.data().height);

        m_skyboxId=calculateSkybox();
        Texture textureIrradiance;
        textureIrradiance.data(texture.data());
        textureIrradiance.id(m_skyboxId);
        PipelineDiffuseIrradiance::getInstance().texture(textureIrradiance);
        PipelinePrefilter::getInstance().texture(textureIrradiance);
        PipelineLUT::getInstance().texture();

    }
}
