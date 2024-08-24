#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include <memory>
#include "../Containers/VAO.h"
#include <chrono>

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
		std::shared_ptr<Prisma::Mesh> m_mesh;

		std::shared_ptr<Prisma::VAO> m_vao;

		Prisma::Mesh::VerticesData m_verticesData;

		unsigned int m_modelPos;

		unsigned int m_cameraPos;

		unsigned int m_lightPos;

		unsigned int m_timePos;

		unsigned int m_bboxMinPos;

		unsigned int m_bboxMaxPos;

		unsigned int m_inverseModelPos;

		std::chrono::system_clock::time_point m_start;

		std::shared_ptr<Prisma::FBO> m_fbo;

	};
}