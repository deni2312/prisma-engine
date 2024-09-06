#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Containers/FBO.h"
#include "../Pipelines/PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include <memory>
#include "../Containers/VAO.h"
#include <chrono>

namespace Prisma {
	class CloudComponent : public Component{
	public:
		CloudComponent();

		void updateRender() override;

		void start() override;

	private:
		unsigned int m_modelPos;

		unsigned int m_cameraPos;

		unsigned int m_lightPos;

		unsigned int m_timePos;

		unsigned int m_resolutionPos;

		unsigned int m_noisePos;

		glm::vec2 m_resolution;

		std::chrono::system_clock::time_point m_start;

		std::shared_ptr<Prisma::FBO> m_fbo;

		std::shared_ptr<Prisma::Texture> m_textureNoise;

		void generateNoise();

	};
}