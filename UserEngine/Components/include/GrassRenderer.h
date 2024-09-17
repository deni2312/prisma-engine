#pragma once
#include <memory>
#include "../../../Engine/include/Containers/Texture.h"
#include "../../../Engine/include/Helpers/Shader.h"
#include "../../../Engine/include/Containers/SSBO.h"
#include "../../../Engine/include/SceneObjects/Mesh.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "../../../Engine/include/Helpers/PrismaRender.h"


class GrassRenderer {
public:
    void start(std::shared_ptr<Prisma::Texture> heightMap);

    unsigned int renderGrass(glm::mat4 translation);

	void generateGrassPoints(float density,float mult,float shift);

private:
	std::shared_ptr<Prisma::Texture> m_grassSprite = nullptr;
    std::shared_ptr<Prisma::Texture> m_heightMap = nullptr;
	std::shared_ptr<Prisma::Shader> m_spriteShader;
	std::shared_ptr<Prisma::Shader> m_cullShader;

	unsigned int m_spritePos;
	unsigned int m_spriteModelPos;

	unsigned int m_modelComputePos;


	std::shared_ptr<Prisma::SSBO> m_ssbo;
	std::shared_ptr<Prisma::SSBO> m_ssboCull;
	std::vector<glm::vec4> m_positions;

	glm::mat4 m_spriteModel = glm::mat4(1.0);
	glm::mat4 m_spriteModelRotation = glm::mat4(1.0);
	std::vector<Prisma::Mesh::Vertex> m_grassVertices;
};