#include "../../include/Pipelines/PipelineDeferred.h"
#include "../../include/SceneData/MeshIndirect.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/SceneObjects/Mesh.h"
#include "../../include/Pipelines/PipelineSkybox.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Pipelines/PipelinePrefilter.h"
#include "../../include/Pipelines/PipelineLUT.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "../../include/Helpers/IBLBuilder.h"
#include <memory>
#include <iostream>
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/ClusterCalculation.h"


Prisma::PipelineDeferred::PipelineDeferred(const unsigned int& width, const unsigned int& height, bool srgb):m_width{ width },m_height{ height }
{
    m_shader = std::make_shared<Shader>("../../../Engine/Shaders/DeferredPipeline/vertex.glsl", "../../../Engine/Shaders/DeferredPipeline/fragment.glsl");
    m_shaderD = std::make_shared<Shader>("../../../Engine/Shaders/DeferredPipeline/vertex_d.glsl", "../../../Engine/Shaders/DeferredPipeline/fragment_d.glsl");
    m_ssr=std::make_shared<Prisma::PipelineSSR>();
    m_shaderD->use();
    m_irradiancePos = m_shaderD->getUniformPosition("irradianceMap");
    m_prefilterPos = m_shaderD->getUniformPosition("prefilterMap");
    m_lutPos = m_shaderD->getUniformPosition("brdfLUT");

    glGenFramebuffers(1, &m_gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    unsigned int gPosition, gNormal, gAlbedoSpec;

    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    m_position = glGetTextureHandleARB(gPosition);
    glMakeTextureHandleResidentARB(m_position);

    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    m_normal = glGetTextureHandleARB(gNormal);
    glMakeTextureHandleResidentARB(m_normal);

    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    m_albedo = glGetTextureHandleARB(gAlbedoSpec);
    glMakeTextureHandleResidentARB(m_albedo);
    
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_shaderD->use();
    m_positionLocation = m_shaderD->getUniformPosition("gPosition");
    m_normalLocation = m_shaderD->getUniformPosition("gNormal");
    m_albedoLocation = m_shaderD->getUniformPosition("gAlbedo");
    m_viewPosLocation = m_shaderD->getUniformPosition("viewPos");

    m_nearPos = m_shaderD->getUniformPosition("zNear");
    m_farPos = m_shaderD->getUniformPosition("zFar");
    m_gridSizePos = m_shaderD->getUniformPosition("gridSize");
    m_screenDimensionsPos = m_shaderD->getUniformPosition("screenDimensions");

    m_settings = Prisma::SettingsLoader::instance().getSettings();

    Prisma::FBO::FBOData fboData;
	fboData.width = m_width;
	fboData.height = m_height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	m_fbo = std::make_shared<Prisma::FBO>(fboData);

    m_fboSSR = std::make_shared<Prisma::FBO>(fboData);

    m_fullscreenPipeline = std::make_shared<Prisma::PipelineFullScreen>();

}

void Prisma::PipelineDeferred::projection(glm::mat4 projection)
{
    m_projection = projection;
}

void Prisma::PipelineDeferred::render(std::shared_ptr<Camera> camera)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_shader->use();

    const auto& indirectLoaded = Prisma::MeshIndirect::getInstance().indirectLoaded();

    glBindVertexArray(indirectLoaded.m_vao);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectLoaded.m_drawBuffer);
    // Call glMultiDrawElementsIndirect to render
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLuint>(currentGlobalScene->meshes.size()), 0);

    // Unbind the buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    m_fbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    m_shaderD->use();
    m_shaderD->setInt64(m_albedoLocation, m_albedo);
    m_shaderD->setInt64(m_normalLocation, m_normal);
    m_shaderD->setInt64(m_positionLocation, m_position);
    m_shaderD->setInt64(m_irradiancePos, Prisma::PipelineDiffuseIrradiance::getInstance().id());
    m_shaderD->setInt64(m_prefilterPos, Prisma::PipelinePrefilter::getInstance().id());
    m_shaderD->setInt64(m_lutPos, Prisma::PipelineLUT::getInstance().id());
    m_shaderD->setVec3(m_viewPosLocation, camera->position());


    m_shaderD->setFloat(m_nearPos, m_settings.nearPlane);
    m_shaderD->setFloat(m_farPos, m_settings.farPlane);
    m_shaderD->setFloat(m_nearPos, m_settings.nearPlane);
    m_shaderD->setUVec3(m_gridSizePos, Prisma::ClusterCalculation::grids());
    m_shaderD->setUVec2(m_screenDimensionsPos, { m_settings.width,m_settings.height });
    Prisma::IBLBuilder::getInstance().renderQuad();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo->frameBufferID());
    glBlitFramebuffer(
        0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    Prisma::PipelineSkybox::getInstance().render(camera);
    m_fbo->unbind();

    /*m_fboSSR->bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    m_ssr->update(m_fbo->texture(),m_albedo,m_normal);
    m_fboSSR->unbind();*/

    m_output->bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    m_fullscreenPipeline->render(m_fbo->texture());

    m_output->unbind();

}

void Prisma::PipelineDeferred::outputFbo(std::shared_ptr<Prisma::FBO> output)
{
    m_output = output;
}

Prisma::PipelineDeferred::~PipelineDeferred()
{
}
