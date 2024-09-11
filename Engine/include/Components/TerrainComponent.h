#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../Containers/FBO.h"
#include <memory>
#include <chrono>
#include "../Containers/Texture.h"
#include "../Containers/VAO.h"
#include "../SceneObjects/Mesh.h"

namespace Prisma {
	class TerrainComponent : public Component {
	public:
		TerrainComponent();

		void ui() override;

		void updateRender(std::shared_ptr<Prisma::FBO> fbo = 0) override;

		void start() override;

		void heightMap(std::shared_ptr<Prisma::Texture> heightMap);
	private:
		std::shared_ptr<Prisma::Texture> m_heightMap = nullptr;
		std::shared_ptr<Prisma::Mesh> m_mesh;
		Prisma::VAO m_vao;
		std::shared_ptr<Prisma::Shader> m_shader;
		unsigned int m_modelPos;
		unsigned int m_heightPos;
		unsigned int m_numPatches = 4;
		unsigned int m_resolution = 20;
	};
}