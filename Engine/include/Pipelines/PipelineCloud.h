#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include <memory>

namespace Prisma {
	class PipelineCloud {
	public:
		PipelineCloud();
		void render();
		PipelineCloud(const PipelineCloud&) = delete;
		PipelineCloud& operator=(const PipelineCloud&) = delete;

		static PipelineCloud& Prisma::PipelineCloud::getInstance()
		{
			if (!instance) {
				instance = std::make_shared<PipelineCloud>();
			}
			return *instance;
		}

	private:
		static std::shared_ptr<PipelineCloud> instance;
		std::shared_ptr<Prisma::Shader> m_shader;
		std::shared_ptr<Prisma::Node> m_root;

	};
}