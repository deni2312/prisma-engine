
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

struct PrivateDataForward {

};

std::shared_ptr<PrivateDataForward> dataForward;

Prisma::PipelineForward::PipelineForward(const unsigned int& width, const unsigned int& height, bool srgb) : m_width{ width }, m_height{ height }
{
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/ForwardPipeline/vertex.glsl", "../../../Engine/Shaders/ForwardPipeline/fragment.glsl");
	m_shaderAnimate = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_forward.glsl", "../../../Engine/Shaders/ForwardPipeline/fragment.glsl");
	Prisma::FBO::FBOData fboData;
	fboData.width = m_width;
	fboData.height = m_height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	m_shader->use();
	m_irradiancePos = m_shader->getUniformPosition("irradianceMap");
	m_prefilterPos = m_shader->getUniformPosition("prefilterMap");
	m_lutPos = m_shader->getUniformPosition("brdfLUT");
    m_viewPos = m_shader->getUniformPosition("viewPos");
	m_irradiancePos = m_shader->getUniformPosition("irradianceMap");

	m_nearPos = m_shader->getUniformPosition("zNear");
	m_farPos = m_shader->getUniformPosition("zFar");
	m_gridSizePos = m_shader->getUniformPosition("gridSize");
	m_screenDimensionsPos = m_shader->getUniformPosition("screenDimensions");

	m_settings = Prisma::SettingsLoader::instance().getSettings();

    m_fbo = std::make_shared<Prisma::FBO>(fboData);
	m_fullscreenPipeline = std::make_shared<Prisma::PipelineFullScreen>();

	m_shaderAnimate->use();

	m_irradianceAnimatePos = m_shader->getUniformPosition("irradianceMap");
	m_prefilterAnimatePos = m_shader->getUniformPosition("prefilterMap");
	m_lutAnimatePos = m_shader->getUniformPosition("brdfLUT");
	m_viewAnimatePos = m_shader->getUniformPosition("viewPos");
	m_irradianceAnimatePos = m_shader->getUniformPosition("irradianceMap");

	m_nearAnimatePos = m_shader->getUniformPosition("zNear");
	m_farAnimatePos = m_shader->getUniformPosition("zFar");
	m_gridSizeAnimatePos = m_shader->getUniformPosition("gridSize");
	m_screenDimensionsAnimatePos = m_shader->getUniformPosition("screenDimensions");

	for (unsigned int i = 0; i < MAX_BONES; ++i)
		m_bonesPos.push_back(m_shaderAnimate->getUniformPosition("finalBonesMatrices[" + std::to_string(i) + "]"));

#ifndef NPHYSICS_DEBUG
    drawDebugger=new DrawDebugger();
    Physics::getInstance().physicsWorld()->dynamicsWorld->setDebugDrawer(drawDebugger);
    Physics::getInstance().physicsWorld()->dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
#endif
}

void Prisma::PipelineForward::projection(glm::mat4 projection)
{
	m_projection = projection;
	MeshHandler::getInstance().data()->projection = m_projection;
	MeshHandler::getInstance().ubo()->modifyData(Prisma::MeshHandler::PROJECTION_OFFSET,sizeof(glm::mat4),glm::value_ptr(m_projection));
}

void Prisma::PipelineForward::render(std::shared_ptr<Camera> camera)
{
	m_fbo->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_shader->use();

	m_shader->setInt64(m_irradiancePos, Prisma::PipelineDiffuseIrradiance::getInstance().id());
	m_shader->setInt64(m_prefilterPos, Prisma::PipelinePrefilter::getInstance().id());
	m_shader->setInt64(m_lutPos, Prisma::PipelineLUT::getInstance().id());
    m_shader->setVec3(m_viewPos, camera->position());

	m_shader->setFloat(m_nearPos, m_settings.nearPlane);
	m_shader->setFloat(m_farPos, m_settings.farPlane);
	m_shader->setFloat(m_nearPos, m_settings.nearPlane);
	m_shader->setUVec3(m_gridSizePos, Prisma::ClusterCalculation::grids());
	m_shader->setUVec2(m_screenDimensionsPos, { m_settings.width,m_settings.height });

	Prisma::MeshIndirect::getInstance().renderMeshes();


	m_shaderAnimate->use();

	m_shaderAnimate->setInt64(m_irradianceAnimatePos, Prisma::PipelineDiffuseIrradiance::getInstance().id());
	m_shaderAnimate->setInt64(m_prefilterAnimatePos, Prisma::PipelinePrefilter::getInstance().id());
	m_shaderAnimate->setInt64(m_lutAnimatePos, Prisma::PipelineLUT::getInstance().id());
	m_shaderAnimate->setVec3(m_viewAnimatePos, camera->position());

	m_shaderAnimate->setFloat(m_nearAnimatePos, m_settings.nearPlane);
	m_shaderAnimate->setFloat(m_farAnimatePos, m_settings.farPlane);
	m_shaderAnimate->setFloat(m_nearAnimatePos, m_settings.nearPlane);
	m_shaderAnimate->setUVec3(m_gridSizeAnimatePos, Prisma::ClusterCalculation::grids());
	m_shaderAnimate->setUVec2(m_screenDimensionsAnimatePos, { m_settings.width,m_settings.height });
	auto boneMatrices=currentGlobalScene->animateMeshes[0]->animator()->GetFinalBoneMatrices();

	for (unsigned int i = 0; i < MAX_BONES; ++i)
		m_shaderAnimate->setMat4(m_bonesPos[i], boneMatrices[i]);

	Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

	Prisma::PipelineSkybox::getInstance().render(camera);

#ifndef NPHYSICS_DEBUG
    drawDebugger->line.setMVP(m_projection*camera->matrix());
    Prisma::Physics::getInstance().physicsWorld()->dynamicsWorld->debugDrawWorld();
#endif

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	m_fbo->unbind();
	m_output->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_fullscreenPipeline->render(m_fbo->texture());

	m_output->unbind();
}

void Prisma::PipelineForward::outputFbo(std::shared_ptr<Prisma::FBO> output)
{
	m_output = output;
}

Prisma::PipelineForward::~PipelineForward()
{
}
