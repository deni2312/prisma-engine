#pragma once

#include "PipelineForward.h"
#include "../SceneObjects/Camera.h"
#include "PipelineDeferred.h"
#include "../Helpers/Settings.h"
#include "../../../GUI/include/ImGuiDebug.h"
#include "PipelineSkybox.h"
#include <memory>

namespace Prisma {
	class PipelineHandler {
	public:
		bool initScene(std::shared_ptr<Prisma::Camera> camera, std::shared_ptr<Prisma::Scene> scene, Prisma::Settings settings, Prisma::SceneLoader::SceneParameters sceneParameters, std::shared_ptr<Prisma::ImguiDebug> imguiDebug);
		std::shared_ptr<Prisma::PipelineForward> forward();
		std::shared_ptr<Prisma::PipelineDeferred> deferred();
	private:
		std::shared_ptr<Prisma::PipelineForward> m_forwardPipeline;
		std::shared_ptr<Prisma::PipelineDeferred> m_deferredPipeline;
	};
}