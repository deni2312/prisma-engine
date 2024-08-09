#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include <memory>
#include "../Containers/FBO.h"

namespace Prisma {
	class PipelinePrePass {
	public:
		PipelinePrePass();
		void render(std::shared_ptr<Prisma::FBO> fbo);
	private:
		std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Shader> m_shaderAnimate;

		std::shared_ptr<Prisma::FBO> m_fbo;

	};
}