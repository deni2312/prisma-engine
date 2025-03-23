#pragma once

#include "../SceneObjects/Camera.h"
#include <memory>

namespace Prisma
{
	class PipelinePrePass
	{
	public:
		PipelinePrePass();
		void render();

	private:
		//std::shared_ptr<Shader> m_shader;
		//std::shared_ptr<Shader> m_shaderAnimate;
	};
}
