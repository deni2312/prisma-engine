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
#define NPHYSICS_DEBUG

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
		unsigned int m_irradiancePos;
		unsigned int m_prefilterPos;
		unsigned int m_lutPos;
        unsigned int m_viewPos;
		unsigned int m_planesCSM;

		unsigned int m_nearPos;
		unsigned int m_farPos;
		unsigned int m_gridSizePos;
		unsigned int m_screenDimensionsPos;

		unsigned int m_irradianceAnimatePos;
		unsigned int m_prefilterAnimatePos;
		unsigned int m_lutAnimatePos;
		unsigned int m_viewAnimatePos;

		unsigned int m_nearAnimatePos;
		unsigned int m_farAnimatePos;
		unsigned int m_gridSizeAnimatePos;
		unsigned int m_screenDimensionsAnimatePos;

		std::shared_ptr<SSBO> m_ssboAnimation;

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