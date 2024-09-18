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
    void start(Prisma::Texture heightMap);

    void renderGrass(glm::mat4 translation);

	void generateGrassPoints(float density,float mult,float shift);

private:
	struct DrawElementsIndirectCommandPadded
	{
		unsigned int  count;
		unsigned int  instanceCount;
		unsigned int  firstIndex;
		unsigned int  baseVertex;
		unsigned int  baseInstance;
		glm::ivec3 padding;
	};
	std::shared_ptr<Prisma::Texture> m_grassSprite = nullptr;
	Prisma::VAO m_vao;
	Prisma::Texture m_heightMap;
	std::shared_ptr<Prisma::Shader> m_spriteShader;
	std::shared_ptr<Prisma::Shader> m_cullShader;

	unsigned int m_spritePos;
	unsigned int m_spriteModelPos;

	unsigned int m_modelComputePos;


	std::shared_ptr<Prisma::SSBO> m_ssbo;
	std::shared_ptr<Prisma::SSBO> m_ssboCull;
	std::vector<glm::vec4> m_positions;

	std::vector<glm::mat4> m_spriteModelRotation;
	std::vector<Prisma::Mesh::Vertex> m_grassVertices;
	Prisma::Mesh::VerticesData m_verticesData;
	std::shared_ptr<Prisma::Mesh> m_grassMesh;

	Prisma::DrawElementsIndirectCommand m_command;
	unsigned int m_indirectId;
};