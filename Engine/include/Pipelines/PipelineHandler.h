#pragma once

#include "PipelineForward.h"
#include "../SceneObjects/Camera.h"
#include "PipelineDeferred.h"
#include "../Helpers/Settings.h"
#include "../../../GUI/include/ImGuiDebug.h"
#include "PipelineSkybox.h"
#include <memory>
#include "PipelineDeferredForward.h"

namespace Prisma {
	class PipelineHandler {
	public:
		bool initScene(Prisma::SceneLoader::SceneParameters sceneParameters);
		std::shared_ptr<Prisma::PipelineForward> forward();
		std::shared_ptr<Prisma::PipelineDeferred> deferred();
		std::shared_ptr<Prisma::PipelineDeferredForward> deferredForward();
	private:
		std::shared_ptr<Prisma::PipelineForward> m_forwardPipeline;
		std::shared_ptr<Prisma::PipelineDeferred> m_deferredPipeline;
		std::shared_ptr<Prisma::PipelineDeferredForward> m_deferredForwardPipeline;
	};
}