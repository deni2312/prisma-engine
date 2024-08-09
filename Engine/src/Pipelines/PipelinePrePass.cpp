#include "../../include/Pipelines/PipelinePrePass.h"
#include "../../include/SceneData/MeshIndirect.h"

Prisma::PipelinePrePass::PipelinePrePass()
{
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/PrePassPipeline/vertex.glsl", "../../../Engine/Shaders/PrePassPipeline/fragment.glsl");
	m_shaderAnimate = std::make_shared<Shader>("../../../Engine/Shaders/PrePassPipeline/vertex_animate.glsl", "../../../Engine/Shaders/PrePassPipeline/fragment.glsl");

}

void Prisma::PipelinePrePass::render()
{
	m_shader->use();
	Prisma::MeshIndirect::getInstance().renderMeshes();

	m_shaderAnimate->use();
	Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

}
