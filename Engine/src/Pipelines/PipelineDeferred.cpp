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
#include "../../include/Helpers/PrismaRender.h"
#include <memory>
#include <iostream>
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/ClusterCalculation.h"
#include "../../../GUI/include/TextureInfo.h"
#include "../../include/Postprocess/Postprocess.h"
#include "../../include/engine.h"
#include "../../include/Handlers/ComponentsHandler.h"


Prisma::PipelineDeferred::PipelineDeferred(const unsigned int& width, const unsigned int& height,
                                           bool srgb): m_width{width}, m_height{height}
{
	Shader::ShaderHeaders header;
	header.fragment = "#version 460 core\n#extension GL_ARB_bindless_texture : enable\n";
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/DeferredPipeline/vertex.glsl",
	                                    "../../../Engine/Shaders/DeferredPipeline/fragment.glsl", nullptr, header);
	m_shaderD = std::make_shared<Shader>("../../../Engine/Shaders/DeferredPipeline/vertex_d.glsl",
	                                     "../../../Engine/Shaders/DeferredPipeline/fragment_d.glsl");
	header.fragment = "#version 460 core\n#extension GL_ARB_bindless_texture : enable\n#define ANIMATE 1\n";
	m_shaderAnimate = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_deferred.glsl",
	                                           "../../../Engine/Shaders/DeferredPipeline/fragment.glsl", nullptr,
	                                           header);

	m_ssr = std::make_shared<PipelineSSR>();
	m_shaderD->use();

	std::vector<Prisma::FBO::FBOData> fboDataBuffer;

	fboDataBuffer.push_back({
		m_width, m_height,GL_RGBA16F,GL_FLOAT, true, false, true, false,GL_NEAREST,GL_CLAMP_TO_BORDER, "POSITION"
	});

	fboDataBuffer.push_back({
		m_width, m_height,GL_RGBA16F,GL_FLOAT, false, false, true, false,GL_NEAREST,GL_CLAMP_TO_BORDER, "NORMAL"
	});

	fboDataBuffer.push_back({
		m_width, m_height,GL_RGBA16F,GL_FLOAT, false, false, true, false,GL_NEAREST,GL_CLAMP_TO_BORDER, "ALBEDO"
	});


	fboDataBuffer.push_back({
		m_width, m_height,GL_RGBA16F,GL_FLOAT, false, false, true, false,GL_NEAREST,GL_CLAMP_TO_BORDER, "AMBIENT"
	});
	m_fboBuffer = std::make_shared<Prisma::FBO>(fboDataBuffer);

	auto textureList = m_fboBuffer->textures();


	m_position = textureList[0];

	m_normal = textureList[1];

	m_albedo = textureList[2];

	m_ambient = textureList[3];

	m_depth = m_fboBuffer->depth();
	m_shaderD->use();
	m_positionLocation = m_shaderD->getUniformPosition("gPosition");
	m_normalLocation = m_shaderD->getUniformPosition("gNormal");
	m_albedoLocation = m_shaderD->getUniformPosition("gAlbedo");
	m_ambientLocation = m_shaderD->getUniformPosition("gAmbient");


	FBO::FBOData fboData;
	fboData.width = m_width;
	fboData.height = m_height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	m_fbo = std::make_shared<FBO>(fboData);

	m_fullscreenPipeline = std::make_shared<PipelineFullScreen>();
}

void Prisma::PipelineDeferred::render()
{
	m_fboBuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_shader->use();

	MeshIndirect::getInstance().renderMeshes();

	m_shaderAnimate->use();

	MeshIndirect::getInstance().renderAnimateMeshes();

	// Unbind the buffer
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	m_fbo->bind();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	ComponentsHandler::getInstance().updatePreRender(m_fbo);

	m_shaderD->use();
	m_shaderD->setInt64(m_albedoLocation, m_albedo);
	m_shaderD->setInt64(m_normalLocation, m_normal);
	m_shaderD->setInt64(m_positionLocation, m_position);
	m_shaderD->setInt64(m_ambientLocation, m_ambient);
	PrismaRender::getInstance().renderQuad();

	//COPY DEPTH FOR SKYBOX AND SPRITES
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboBuffer->frameBufferID());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo->frameBufferID());
	glBlitFramebuffer(
		0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST
	);
	PipelineSkybox::getInstance().render();

	ComponentsHandler::getInstance().updateRender(m_fbo);
	ComponentsHandler::getInstance().updatePostRender(m_fbo);
	for (auto& sprite : Prisma::GlobalData::getInstance().currentGlobalScene()->sprites)
	{
		sprite->render();
	}
	Physics::getInstance().drawDebug();

	m_fbo->unbind();

	Postprocess::getInstance().fboRaw(m_fbo);
	uint64_t finalTexture = m_fbo->texture();
	Postprocess::getInstance().fbo(Prisma::GlobalData::getInstance().fboTarget());
	if (Engine::getInstance().engineSettings().ssr)
	{
		m_ssr->update(m_albedo, m_position, m_normal, m_fbo->texture(), m_depth);

		const auto& ssrTexture = m_ssr->texture();

		finalTexture = ssrTexture->texture();
		Postprocess::getInstance().fboRaw(ssrTexture);
	}
	if (Prisma::GlobalData::getInstance().fboTarget())
	{
		Prisma::GlobalData::getInstance().fboTarget()->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_fullscreenPipeline->render(finalTexture);

		Prisma::GlobalData::getInstance().fboTarget()->unbind();
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_fullscreenPipeline->render(m_fbo->texture());
	}
}

Prisma::PipelineDeferred::~PipelineDeferred()
{
}
