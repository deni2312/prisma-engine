#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include "../Physics/DrawDebugger.h"
#include <memory>

namespace Prisma {
	class PipelineForward {
	public:
		PipelineForward(const unsigned int& width=1920, const unsigned int& height=1080,bool srgb=true);
		void projection(glm::mat4 projection);
		void render(std::shared_ptr<Camera> camera);
		void outputFbo(std::shared_ptr<Prisma::FBO> output);
		~PipelineForward();
	private:
		unsigned int m_width;
		unsigned int m_height;

		std::vector<unsigned int> m_cascadePlaneDistances;

		Prisma::Settings m_settings;

		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Shader> m_shaderAnimate;
		glm::mat4 m_projection;
		std::shared_ptr<Prisma::FBO> m_fbo;
		std::shared_ptr<Prisma::FBO> m_output;
		std::shared_ptr<Prisma::PipelineFullScreen> m_fullscreenPipeline;
#ifndef NPHYSICS_DEBUG
        DrawDebugger* drawDebugger;
#endif

    };
}