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
		std::shared_ptr<Prisma::Texture> m_grass = nullptr;
		std::shared_ptr<Prisma::Texture> m_stone = nullptr;
		std::shared_ptr<Prisma::Texture> m_snow = nullptr;
		std::shared_ptr<Prisma::Mesh> m_mesh;
		Prisma::VAO m_vao;
		std::shared_ptr<Prisma::Shader> m_shader;
		unsigned int m_modelPos;
		unsigned int m_heightPos;
		unsigned int m_multPos;
		unsigned int m_shiftPos;
		unsigned int m_minPos;
		unsigned int m_maxPos;
		unsigned int m_grassPos;
		unsigned int m_stonePos;
		unsigned int m_snowPos;
		unsigned int m_scalePos;
		unsigned int m_numPatches = 4;
		unsigned int m_resolution = 20;
		float m_mult = 64;
		float m_shift = -16;
		float m_min = 20;
		float m_max = 1000;
		float m_scale = 1000;
	};
}