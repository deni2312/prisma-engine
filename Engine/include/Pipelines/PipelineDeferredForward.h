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

namespace Prisma
{
	class PipelineDeferredForward
	{
	public:
		PipelineDeferredForward(const unsigned int& width, const unsigned int& height, bool srgb);
		void render();
		~PipelineDeferredForward();

	private:
		unsigned int m_width;
		unsigned int m_height;
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Shader> m_shaderD;
		std::shared_ptr<Shader> m_shaderAnimate;
		std::shared_ptr<Shader> m_shaderCompute;
		unsigned int m_gBuffer;
		uint64_t m_position;
		uint64_t m_normal;
		uint64_t m_albedo;
		uint64_t m_depth;
		uint64_t m_ambient;
		unsigned int m_positionLocation;
		unsigned int m_normalLocation;
		unsigned int m_albedoLocation;
		unsigned int m_ambientLocation;
		unsigned int m_transparentLocation;
		std::shared_ptr<FBO> m_fbo;
		std::shared_ptr<PipelineFullScreen> m_fullscreenPipeline;

		std::shared_ptr<PipelineSSR> m_ssr;
	};
}
