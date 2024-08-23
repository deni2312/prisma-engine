#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include <memory>
#include "../Containers/VAO.h"

namespace Prisma {
	class PipelineCloud {
	public:
		PipelineCloud();
		void render();
		PipelineCloud(const PipelineCloud&) = delete;
		PipelineCloud& operator=(const PipelineCloud&) = delete;

		static PipelineCloud& getInstance();

	private:
		static std::shared_ptr<PipelineCloud> instance;
		std::shared_ptr<Prisma::Shader> m_shader;
		std::shared_ptr<Prisma::Node> m_root;

		std::shared_ptr<Prisma::VAO> m_vao;

		Prisma::Mesh::VerticesData m_verticesData;

		unsigned int m_modelPos;

	};
}