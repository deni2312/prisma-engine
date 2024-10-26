#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Pipelines/PipelineForward.h"
#include "../../include/Helpers/PrismaRender.h"
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
#include "../../include/Handlers/ComponentsHandler.h"


Prisma::PipelineForward::PipelineForward(const unsigned int& width, const unsigned int& height, bool srgb) : m_width{ width }, m_height{ height }
{
	Shader::ShaderHeaders header;
	header.fragment = "#version 460 core\n#extension GL_ARB_bindless_texture : enable\n";

	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/ForwardPipeline/vertex.glsl", "../../../Engine/Shaders/ForwardPipeline/fragment.glsl",nullptr,header);
	
	header.fragment = "#version 460 core\n#extension GL_ARB_bindless_texture : enable\n#define ANIMATE 1\n";

	m_shaderAnimate = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_forward.glsl", "../../../Engine/Shaders/ForwardPipeline/fragment.glsl",nullptr,header);

	m_shaderTransparent = std::make_shared<Shader>("../../../Engine/Shaders/TransparentPipeline/compute.glsl");
	Prisma::FBO::FBOData fboData;
	fboData.width = m_width;
	fboData.height = m_height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	fboData.enableMultisample = true;


    m_fbo = std::make_shared<Prisma::FBO>(fboData);
	fboData.enableMultisample = false;
	fboData.rbo = false;


	m_fboCopy = std::make_shared<Prisma::FBO>(fboData);
	m_shader->use();
	m_fullscreenPipeline = std::make_shared<Prisma::PipelineFullScreen>();

	m_prepass = std::make_shared<Prisma::PipelinePrePass>();

}

void Prisma::PipelineForward::render()
{
	m_fbo->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
/*
	//DEPTH PREPASS
	m_prepass->render();

	// After depth pre-pass
	glDepthMask(GL_FALSE);          // Disable depth writing
	glDepthFunc(GL_LEQUAL);         // Ensure correct depth testing for subsequent passes
	*/
	/*glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Prisma::ComponentsHandler::getInstance().updatePreRender(m_fbo);
	//COLOR PASS
	auto sizeMeshes = currentGlobalScene->meshes.size() + currentGlobalScene->animateMeshes.size();
	m_shaderTransparent->use();
	m_shaderTransparent->dispatchCompute({ 1,1,1});
	m_shaderTransparent->wait(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
	*/
	m_shaderAnimate->use();
	Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

	m_shader->use();
	Prisma::MeshIndirect::getInstance().renderMeshes();

	Prisma::PipelineSkybox::getInstance().render();
	//glDisable(GL_BLEND);

	Prisma::ComponentsHandler::getInstance().updateRender(m_fbo);

	for (auto& sprite : currentGlobalScene->sprites) {
		sprite->render();
	}


/*
	// After rendering
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
*/

	Prisma::Physics::getInstance().drawDebug();

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
