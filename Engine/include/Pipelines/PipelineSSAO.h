#pragma once

#include <memory>
#include "../Helpers/Shader.h"
#include "../Containers/FBO.h"

namespace Prisma
{
	class PipelineSSAO
	{
	public:
		PipelineSSAO();

		void update(uint64_t depth);

		std::shared_ptr<FBO> texture();

	private:
		float ourLerp(float a, float b, float f);
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<FBO> m_fboBlur;
		unsigned int m_depthPos;
	};
}
