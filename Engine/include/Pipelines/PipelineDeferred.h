#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "PipelineFullScreen.h"
#include "PipelineSSR.h"
#include <memory>
#include "../Helpers/Settings.h"

namespace Prisma
{
	class PipelineDeferred
	{
	public:
		struct DeferredData
		{
			uint64_t position;
			uint64_t normal;
			uint64_t albedo;
			uint64_t depth;
			uint64_t ambient;
		};

		PipelineDeferred(const unsigned int& width, const unsigned int& height, bool srgb);
		void render();
		~PipelineDeferred();

	private:
		unsigned int m_width;
		unsigned int m_height;
		//std::shared_ptr<Shader> m_shader;
		//std::shared_ptr<Shader> m_shaderD;
		//std::shared_ptr<Shader> m_shaderAnimate;

		DeferredData m_deferredData;

		unsigned int m_positionLocation;
		unsigned int m_normalLocation;
		unsigned int m_albedoLocation;
		unsigned int m_ambientLocation;
		//std::shared_ptr<FBO> m_fboBuffer;
		//std::shared_ptr<FBO> m_fbo;
		std::shared_ptr<PipelineFullScreen> m_fullscreenPipeline;

		std::shared_ptr<PipelineSSR> m_ssr;
	};
}
