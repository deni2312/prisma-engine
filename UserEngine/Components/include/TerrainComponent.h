#pragma once

#include <memory>
#include <chrono>
#include <functional>
#include "../../../Engine/include/Containers/Texture.h"
#include "../../../Engine/include/SceneObjects/Mesh.h"
#include "GrassRenderer.h"

namespace Prisma
{
	class TerrainComponent : public Component
	{
	public:
		TerrainComponent();

		void ui() override;

		//void updateRender(std::shared_ptr<FBO> fbo = nullptr) override;

		void generateCpu();

		void start() override;

		void heightMap(Texture heightMap);

	private:
		Texture m_heightMap;
		std::shared_ptr<Texture> m_grass = nullptr;
		std::shared_ptr<Texture> m_stone = nullptr;
		std::shared_ptr<Texture> m_snow = nullptr;
		std::shared_ptr<Texture> m_grassNormal = nullptr;
		std::shared_ptr<Texture> m_stoneNormal = nullptr;
		std::shared_ptr<Texture> m_snowNormal = nullptr;
		std::shared_ptr<Texture> m_grassRoughness = nullptr;
		std::shared_ptr<Texture> m_stoneRoughness = nullptr;
		std::shared_ptr<Texture> m_snowRoughness = nullptr;

		std::shared_ptr<Mesh> m_mesh;
		//VAO m_vao;
		//std::shared_ptr<Shader> m_shader;
		//std::shared_ptr<Shader> m_csmShader;

		unsigned int m_modelPos;
		unsigned int m_heightPos;
		unsigned int m_multPos;
		unsigned int m_shiftPos;
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
		float m_mult = 8;
		float m_shift = 0;
		float m_scale = 1000;
		float m_farPlane = 0;

		std::function<void()> m_startButton;
		std::function<void()> m_apply;

		void generatePhysics();

		GrassRenderer m_grassRenderer;

		unsigned int m_strips;
		unsigned int m_stripTris;

		std::shared_ptr<Prisma::Mesh::VerticesData> m_vertices;
	};
}
