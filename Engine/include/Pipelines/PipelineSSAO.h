#pragma once

#include <memory>
#include "../Helpers/Shader.h"
#include "../Containers/FBO.h"
#include "../Containers/UBO.h"

namespace Prisma
{
	class PipelineSSAO
	{
	public:
		PipelineSSAO();

		void update(uint64_t depth,uint64_t position);

		std::shared_ptr<FBO> texture();

	private:
		float ourLerp(float a, float b, float f);
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<FBO> m_fbo;
		std::shared_ptr<FBO> m_fboBlur;

		unsigned int m_depthPos;
		uint64_t m_noise;
		unsigned int m_noisePos;
		unsigned int m_positionPos;
		std::shared_ptr<Prisma::Ubo> m_ubo;
		unsigned int m_noiseScalePos;
		unsigned int m_aspectRatioPos;
		glm::vec2 m_aspectRatio;
	};
}
