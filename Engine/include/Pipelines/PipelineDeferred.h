#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include "PipelineSSR.h"
#include <memory>
#include "../Helpers/Settings.h"

namespace Prisma {
	class PipelineDeferred {
	public:
		PipelineDeferred(const unsigned int& width,const unsigned int& height,bool srgb);
		void projection(glm::mat4 projection);
		void render(std::shared_ptr<Camera> camera);
		void outputFbo(std::shared_ptr<Prisma::FBO> output);
		~PipelineDeferred();
	private:
		unsigned int m_width;
		unsigned int m_height;
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Shader> m_shaderD;
		glm::mat4 m_projection;
		unsigned int m_gBuffer;
		uint64_t m_position;
		uint64_t m_normal;
		uint64_t m_albedo;
		unsigned int m_positionLocation;
		unsigned int m_normalLocation;
		unsigned int m_albedoLocation;
		unsigned int m_viewPosLocation;
		std::shared_ptr<Prisma::FBO> m_output;
		std::shared_ptr<Prisma::FBO> m_fbo;
        std::shared_ptr<Prisma::FBO> m_fboSSR;
        std::shared_ptr<Prisma::PipelineFullScreen> m_fullscreenPipeline;
		unsigned int m_irradiancePos;
		unsigned int m_prefilterPos;
		unsigned int m_lutPos;

		unsigned int m_nearPos;
		unsigned int m_farPos;
		unsigned int m_gridSizePos;
		unsigned int m_screenDimensionsPos;

		Prisma::Settings m_settings;

        std::shared_ptr<Prisma::PipelineSSR> m_ssr;
    };
}