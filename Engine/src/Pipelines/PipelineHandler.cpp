#include "../../include/Pipelines/PipelineHandler.h"
#include "../../include/Helpers/SettingsLoader.h"

bool Prisma::PipelineHandler::initScene(SceneLoader::SceneParameters sceneParameters)
{
	//if (Prisma::GlobalData::getInstance().currentGlobalScene() && Prisma::GlobalData::getInstance().currentGlobalScene()->camera)
	{
		auto settings = SettingsLoader::getInstance().getSettings();
		m_forwardPipeline = std::make_shared<PipelineForward>(settings.width, settings.height, sceneParameters.srgb);

		//m_deferredPipeline = std::make_shared<PipelineDeferred>(settings.width, settings.height, sceneParameters.srgb);

		//m_deferredForwardPipeline = std::make_shared<PipelineDeferredForward>(settings.width, settings.height, sceneParameters.srgb);
		return true;
	}
	return false;
}

std::shared_ptr<Prisma::PipelineForward> Prisma::PipelineHandler::forward()
{
	return m_forwardPipeline;
}

std::shared_ptr<Prisma::PipelineDeferred> Prisma::PipelineHandler::deferred()
{
	return m_deferredPipeline;
}

std::shared_ptr<Prisma::PipelineDeferredForward> Prisma::PipelineHandler::deferredForward()
{
	return m_deferredForwardPipeline;
}
