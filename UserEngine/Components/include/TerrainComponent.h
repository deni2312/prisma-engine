#pragma once

#include <memory>
#include <chrono>
#include <functional>
#include "../../../Engine/include/Containers/FBO.h"
#include "../../../Engine/include/Containers/Texture.h"
#include "../../../Engine/include/SceneObjects/Mesh.h"
#include "../../../Engine/include/Containers/VAO.h"
#include "../../../Engine/include/Containers/SSBO.h"
#include "GrassRenderer.h"

namespace Prisma {
	class TerrainComponent : public Component {
	public:
		TerrainComponent();

		void ui() override;

		void updateRender(std::shared_ptr<Prisma::FBO> fbo = 0) override;

		void generateCpu();

		void start() override;

		void heightMap(Prisma::Texture heightMap);
	private:
		Prisma::Texture m_heightMap;
		std::shared_ptr<Prisma::Texture> m_grass = nullptr;
		std::shared_ptr<Prisma::Texture> m_stone = nullptr;
		std::shared_ptr<Prisma::Texture> m_snow = nullptr;
		std::shared_ptr<Prisma::Texture> m_grassNormal = nullptr;
		std::shared_ptr<Prisma::Texture> m_stoneNormal = nullptr;
		std::shared_ptr<Prisma::Texture> m_snowNormal = nullptr;
		std::shared_ptr<Prisma::Texture> m_grassRoughness = nullptr;
		std::shared_ptr<Prisma::Texture> m_stoneRoughness = nullptr;
		std::shared_ptr<Prisma::Texture> m_snowRoughness = nullptr;

		std::shared_ptr<Prisma::Mesh> m_mesh;
		Prisma::VAO m_vao;
		std::shared_ptr<Prisma::Shader> m_shader;
		std::shared_ptr<Prisma::Shader> m_csmShader;

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
		unsigned int m_grassNormalPos;
		unsigned int m_stoneNormalPos;
		unsigned int m_snowNormalPos;
		unsigned int m_grassRoughnessPos;
		unsigned int m_stoneRoughnessPos;
		unsigned int m_snowRoughnessPos;

		unsigned int m_numPatches = 4;
		unsigned int m_resolution = 20;
		float m_mult = 64;
		float m_shift = 0;
		float m_min = 20;
		float m_max = 1000;
		float m_scale = 1000;

		std::function<void()> m_startButton;

		void generatePhysics();

		GrassRenderer m_grassRenderer;
	};
}