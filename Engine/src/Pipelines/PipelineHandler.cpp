#include "../../include/Pipelines/PipelineHandler.h"

bool Prisma::PipelineHandler::initScene(std::shared_ptr<Prisma::Camera> camera, std::shared_ptr<Prisma::Scene> scene, Prisma::Settings settings, Prisma::SceneLoader::SceneParameters sceneParameters, std::shared_ptr<Prisma::ImguiDebug> imguiDebug)
{
    if (camera && scene) {

        m_forwardPipeline = std::make_shared<Prisma::PipelineForward>(settings.width, settings.height, sceneParameters.srgb);
        m_forwardPipeline->projection(glm::perspective(glm::radians(settings.angle), (float)settings.width / (float)settings.height, settings.nearPlane, settings.farPlane));
        m_forwardPipeline->outputFbo(imguiDebug->fbo());

        m_deferredPipeline = std::make_shared<Prisma::PipelineDeferred>(settings.width, settings.height, sceneParameters.srgb);
        m_deferredPipeline->projection(glm::perspective(glm::radians(settings.angle), (float)settings.width / (float)settings.height, settings.nearPlane, settings.farPlane));
        m_deferredPipeline->outputFbo(imguiDebug->fbo());
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
