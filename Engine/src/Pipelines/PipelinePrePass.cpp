#include "../../include/Pipelines/PipelinePrePass.h"
#include "../../include/SceneData/MeshIndirect.h"
#include "../../include/Helpers/SettingsLoader.h"

Prisma::PipelinePrePass::PipelinePrePass()
{
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/PrePassPipeline/vertex.glsl", "../../../Engine/Shaders/PrePassPipeline/fragment.glsl");
	m_shaderAnimate = std::make_shared<Shader>("../../../Engine/Shaders/PrePassPipeline/vertex_animate.glsl", "../../../Engine/Shaders/PrePassPipeline/fragment.glsl");
	auto settings = Prisma::SettingsLoader::instance().getSettings();

	Prisma::FBO::FBOData fboData;
	fboData.width = settings.width;
	fboData.height = settings.height;
	fboData.enableDepth = true;
	fboData.rbo = false;

	m_fbo = std::make_shared<Prisma::FBO>(fboData);
}

void Prisma::PipelinePrePass::render(std::shared_ptr<Prisma::FBO> fbo)
{
	m_shader->use();
	Prisma::MeshIndirect::getInstance().renderMeshes();

	m_shaderAnimate->use();
	Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

}
