
#include "../../include/GlobalData/GlobalData.h"

#include "../../include/Pipelines/PipelineForward.h"
#include "../../include/Helpers/IBLBuilder.h"
#include "../../include/SceneData/MeshIndirect.h"
#include "../../include/SceneObjects/Mesh.h"
#include "../../include/Pipelines/PipelineSkybox.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Pipelines/PipelinePrefilter.h"
#include "../../include/Pipelines/PipelineLUT.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <memory>
#include <iostream>
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/ClusterCalculation.h"
#include <random>
#include "../../include/Postprocess/Postprocess.h"

struct PrivateDataForward {

};

std::shared_ptr<PrivateDataForward> dataForward;

Prisma::PipelineForward::PipelineForward(const unsigned int& width, const unsigned int& height, bool srgb) : m_width{ width }, m_height{ height }
{
	Shader::ShaderHeaders header;
	header.fragment = "#version 460 core\n#extension GL_ARB_bindless_texture : enable\n";

	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/ForwardPipeline/vertex.glsl", "../../../Engine/Shaders/ForwardPipeline/fragment.glsl",nullptr,header);
	
	header.fragment = "#version 460 core\n#extension GL_ARB_bindless_texture : enable\n#define ANIMATE 1\n";

	m_shaderAnimate = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_forward.glsl", "../../../Engine/Shaders/ForwardPipeline/fragment.glsl",nullptr,header);
	Prisma::FBO::FBOData fboData;
	fboData.width = m_width;
	fboData.height = m_height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	fboData.enableMultisample = true;

	m_shader->use();

    m_fbo = std::make_shared<Prisma::FBO>(fboData);

	Prisma::FBO::FBOData fboDataCopy;
	fboData.width = m_width;
	fboData.height = m_height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;

	m_fboCopy = std::make_shared<Prisma::FBO>(fboDataCopy);

	m_fullscreenPipeline = std::make_shared<Prisma::PipelineFullScreen>();

#ifndef NPHYSICS_DEBUG
    drawDebugger=new DrawDebugger();
    Physics::getInstance().physicsWorld()->dynamicsWorld->setDebugDrawer(drawDebugger);
    Physics::getInstance().physicsWorld()->dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
#endif
}

void Prisma::PipelineForward::render()
{
	m_fbo->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_shader->use();

	Prisma::MeshIndirect::getInstance().renderMeshes();

	m_shaderAnimate->use();

	Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

	Prisma::PipelineSkybox::getInstance().render();

#ifndef NPHYSICS_DEBUG
    drawDebugger->line.setMVP(currentProjection * currentGlobalScene->camera->matrix());
    Prisma::Physics::getInstance().physicsWorld()->dynamicsWorld->debugDrawWorld();
#endif

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	m_fbo->unbind();


	m_fboCopy->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo->frameBufferID());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboCopy->frameBufferID());
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	m_fboCopy->unbind();


	Prisma::Postprocess::getInstance().fboRaw(m_fboCopy);
	Postprocess::getInstance().fbo(fboTarget);
	if (fboTarget) {
		fboTarget->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_fullscreenPipeline->render(m_fboCopy->texture());
		fboTarget->unbind();
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_fullscreenPipeline->render(m_fboCopy->texture());
	}
}

Prisma::PipelineForward::~PipelineForward()
{
}
