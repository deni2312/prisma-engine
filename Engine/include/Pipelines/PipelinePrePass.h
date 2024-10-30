#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include <memory>
#include "../Containers/FBO.h"

namespace Prisma
{
	class PipelinePrePass
	{
	public:
		PipelinePrePass();
		void render();

	private:
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Shader> m_shaderAnimate;
	};
}
