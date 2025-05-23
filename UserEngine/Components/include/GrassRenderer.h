#pragma once
#include <memory>
#include "Containers/Texture.h"
#include "SceneObjects/Mesh.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "Helpers/PrismaRender.h"
#include <chrono>


class GrassRenderer {
public:
    void start(Prisma::Texture heightMap);

    void renderGrass(glm::mat4 translation);

    void generateGrassPoints(float density, float mult, float shift);

    void projection(glm::mat4 projection);

private:
    struct DrawElementsIndirectCommandPadded {
        unsigned int count;
        unsigned int instanceCount;
        unsigned int firstIndex;
        unsigned int baseVertex;
        unsigned int baseInstance;
        glm::ivec3 padding;
    };

    struct GrassPosition {
        glm::mat4 direction;
        glm::mat4 position;
    };

    std::shared_ptr<Prisma::Texture> m_grassSprite = nullptr;
    //Prisma::VAO m_vao;
    Prisma::Texture m_heightMap;
    //std::shared_ptr<Prisma::Shader> m_spriteShader;
    //std::shared_ptr<Prisma::Shader> m_cullShader;

    unsigned int m_spritePos;
    unsigned int m_spriteModelPos;

    unsigned int m_modelComputePos;
    unsigned int m_percentPos;

    //std::shared_ptr<Prisma::SSBO> m_ssbo;
    //std::shared_ptr<Prisma::SSBO> m_ssboCull;
    std::vector<GrassPosition> m_grassPositions;

    std::vector<Prisma::Mesh::Vertex> m_grassVertices;
    Prisma::Mesh::VerticesData m_verticesData;
    std::shared_ptr<Prisma::Mesh> m_grassMesh;

    //Prisma::DrawElementsIndirectCommand m_command;
    unsigned int m_indirectId;

    float m_percentValue;
    float m_timePos;

    glm::mat4 m_projection;
    unsigned int m_projectionPos;

    std::chrono::steady_clock::time_point m_startPoint;

    void generateGrassPositions(float density, float mult, float shift);
};