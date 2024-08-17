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
#include "../../../GUI/include/TextureInfo.h"
#include "../../include/Postprocess/Postprocess.h"
#include "../../include/engine.h"


Prisma::PipelineDeferred::PipelineDeferred(const unsigned int& width, const unsigned int& height, bool srgb):m_width{ width },m_height{ height }
{
    Shader::ShaderHeaders header;
    header.fragment = "#version 460 core\n#extension GL_ARB_bindless_texture : enable\n";
    m_shader = std::make_shared<Shader>("../../../Engine/Shaders/DeferredPipeline/vertex.glsl", "../../../Engine/Shaders/DeferredPipeline/fragment.glsl",nullptr, header);
    m_shaderD = std::make_shared<Shader>("../../../Engine/Shaders/DeferredPipeline/vertex_d.glsl", "../../../Engine/Shaders/DeferredPipeline/fragment_d.glsl");
    header.fragment = "#version 460 core\n#extension GL_ARB_bindless_texture : enable\n#define ANIMATE 1\n";
    m_shaderAnimate = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_deferred.glsl", "../../../Engine/Shaders/DeferredPipeline/fragment.glsl",nullptr,header);

    m_ssr=std::make_shared<Prisma::PipelineSSR>();
    m_shaderD->use();

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

    unsigned int depthTexture;

    // Generate and configure the depth texture
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    m_depth = glGetTextureHandleARB(depthTexture);
    glMakeTextureHandleResidentARB(m_depth);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_shaderD->use();
    m_positionLocation = m_shaderD->getUniformPosition("gPosition");
    m_normalLocation = m_shaderD->getUniformPosition("gNormal");
    m_albedoLocation = m_shaderD->getUniformPosition("gAlbedo");

    Prisma::TextureInfo::getInstance().add({ gPosition, "Deferred_Position"});
    Prisma::TextureInfo::getInstance().add({ gNormal, "Deferred_Normal"});
    Prisma::TextureInfo::getInstance().add({ gAlbedoSpec, "Deferred_Albedo"});

    Prisma::FBO::FBOData fboData;
	fboData.width = m_width;
	fboData.height = m_height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	m_fbo = std::make_shared<Prisma::FBO>(fboData);

    m_fullscreenPipeline = std::make_shared<Prisma::PipelineFullScreen>();

}

void Prisma::PipelineDeferred::render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_shader->use();

    Prisma::MeshIndirect::getInstance().renderMeshes();

    m_shaderAnimate->use();

    Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

    // Unbind the buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

    m_fbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    m_shaderD->use();
    m_shaderD->setInt64(m_albedoLocation, m_albedo);
    m_shaderD->setInt64(m_normalLocation, m_normal);
    m_shaderD->setInt64(m_positionLocation, m_position);
    Prisma::IBLBuilder::getInstance().renderQuad();

    //COPY DEPTH FOR SKYBOX AND SPRITES
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo->frameBufferID());
    glBlitFramebuffer(
        0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    Prisma::PipelineSkybox::getInstance().render();

    for (auto& sprite : currentGlobalScene->sprites) {
        sprite->render();
    }

    m_fbo->unbind();

    Prisma::Postprocess::getInstance().fboRaw(m_fbo);
    uint64_t finalTexture = m_fbo->texture();
    Postprocess::getInstance().fbo(fboTarget);
    if (Prisma::Engine::getInstance().engineSettings().ssr) {
        m_ssr->update(m_albedo, m_position, m_normal, m_fbo->texture(), m_depth);

        const auto& ssrTexture = m_ssr->texture();

        finalTexture = ssrTexture->texture();
        Prisma::Postprocess::getInstance().fboRaw(ssrTexture);
    }
    if (fboTarget) {
        fboTarget->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_fullscreenPipeline->render(finalTexture);

        fboTarget->unbind();
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_fullscreenPipeline->render(m_fbo->texture());
    }

}

Prisma::PipelineDeferred::~PipelineDeferred()
{
}
