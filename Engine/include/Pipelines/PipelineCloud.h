#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include <memory>
#include "../Containers/VAO.h"
#include "../../include/Postprocess/PostprocessEffect.h"

#include <chrono>

namespace Prisma {
	class PipelineCloud : public Prisma::PostprocessEffect{
	public:
		PipelineCloud();

		virtual void render(std::shared_ptr<Prisma::FBO> texture, std::shared_ptr<Prisma::FBO> raw) override;

	private:
		std::shared_ptr<Prisma::Shader> m_shader;
		std::shared_ptr<Prisma::Mesh> m_mesh;

		std::shared_ptr<Prisma::VAO> m_vao;

		Prisma::Mesh::VerticesData m_verticesData;

		unsigned int m_cameraPos;

		unsigned int m_lightPos;

		unsigned int m_timePos;

		unsigned int m_texturePos;

		std::chrono::system_clock::time_point m_start;

	};
}