#pragma once

#include <memory>

namespace Prisma
{
	class PipelineSSR
	{
	public:
		PipelineSSR();

		void update(uint64_t albedo, uint64_t position, uint64_t normal, uint64_t finalImage, uint64_t depth);

		//std::shared_ptr<FBO> texture();

	private:
		//std::shared_ptr<Shader> m_shader;
		//std::shared_ptr<FBO> m_fboSSR;
		unsigned int m_albedoPos;
		unsigned int m_normalPos;
		unsigned int m_positionPos;
		unsigned int m_finalImagePos;
		unsigned int m_depthPos;
		unsigned int m_invProjPos;
	};
}
