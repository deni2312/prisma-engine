#include "../../include/Pipelines/PipelineHandler.h"
#include "../../include/Helpers/SettingsLoader.h"

bool Prisma::PipelineHandler::initScene(Prisma::SceneLoader::SceneParameters sceneParameters)
{
    if (currentGlobalScene && currentGlobalScene->camera) {
        auto settings = Prisma::SettingsLoader::getInstance().getSettings();
        m_forwardPipeline = std::make_shared<Prisma::PipelineForward>(settings.width, settings.height, sceneParameters.srgb);

        m_deferredPipeline = std::make_shared<Prisma::PipelineDeferred>(settings.width, settings.height, sceneParameters.srgb);
        return true;
    }
    else {
        return false;
    }
}

std::shared_ptr<Prisma::PipelineForward> Prisma::PipelineHandler::forward()
{
    return m_forwardPipeline;
}

std::shared_ptr<Prisma::PipelineDeferred> Prisma::PipelineHandler::deferred()
{
    return m_deferredPipeline;
}
