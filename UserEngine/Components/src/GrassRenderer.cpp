#include "../include/GrassRenderer.h"
#include <random>
#include <glm/gtx/string_cast.hpp>

void GrassRenderer::start(Prisma::Texture heightMap) {
    m_heightMap = heightMap;
    m_spriteShader = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/GrassPipeline/vertex.glsl", "../../../UserEngine/Shaders/GrassPipeline/fragment.glsl");
    m_cullShader = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/GrassPipeline/compute.glsl");
    m_grassSprite = std::make_shared<Prisma::Texture>();
    m_grassSprite->loadTexture("../../../Resources/DefaultScene/sprites/grass.png", false, true, false);
    m_spriteShader->use();
    m_spritePos = m_spriteShader->getUniformPosition("grassSprite");
    m_spriteModelPos = m_spriteShader->getUniformPosition("model");
    m_percentPos = m_spriteShader->getUniformPosition("percent");
    m_timePos = m_spriteShader->getUniformPosition("time");
    m_startPoint = std::chrono::high_resolution_clock::now();
    m_cullShader->use();
    m_modelComputePos = m_cullShader->getUniformPosition("model");
    m_projectionPos = m_cullShader->getUniformPosition("currentProjection");
    m_ssbo = std::make_shared<Prisma::SSBO>(15);
    m_ssboCull = std::make_shared<Prisma::SSBO>(16);

    Prisma::SceneLoader sceneLoader;

    auto grass = sceneLoader.loadScene("../../../Resources/DefaultScene/grass/grass.gltf", { true });
    m_grassMesh = std::dynamic_pointer_cast<Prisma::Mesh>(grass->root->children()[0]);
    m_verticesData = m_grassMesh->verticesData();

    float yMax = m_verticesData.vertices[0].position.y;

    for (const auto& v : m_verticesData.vertices) {
        if (v.position.y > yMax) {
            yMax = v.position.y;
        }
    }
    m_percentValue = yMax / 100.0;

    m_vao.bind();
    Prisma::VBO vbo;

    Prisma::EBO ebo;

    vbo.writeData(m_verticesData.vertices.size() * sizeof(Prisma::Mesh::Vertex), &m_verticesData.vertices[0], GL_DYNAMIC_DRAW);
    ebo.writeData(m_verticesData.indices.size() * sizeof(unsigned int), &m_verticesData.indices[0], GL_DYNAMIC_DRAW);

    m_vao.addAttribPointer(0, 3, sizeof(Prisma::Mesh::Vertex), (void*)0);
    m_vao.addAttribPointer(1, 3, sizeof(Prisma::Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, normal));
    m_vao.addAttribPointer(2, 2, sizeof(Prisma::Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, texCoords));
    m_vao.resetVao();
}

void GrassRenderer::renderGrass(glm::mat4 translation) {
    m_cullShader->use();
    m_cullShader->setMat4(m_modelComputePos, translation);
    m_cullShader->setMat4(m_projectionPos, m_projection);
    unsigned int sizeCluster = 8;
    unsigned int sizePositions = glm::ceil(glm::sqrt(m_positions.size() / (sizeCluster * sizeCluster)));
    m_cullShader->dispatchCompute({ sizePositions,sizePositions,1 });
    m_cullShader->wait(GL_COMMAND_BARRIER_BIT);
    m_vao.bind();
    m_spriteShader->use();
    m_spriteShader->setInt64(m_spritePos, m_grassSprite->id());
    m_spriteShader->setMat4(m_spriteModelPos, translation * m_grassMesh->finalMatrix());
    m_spriteShader->setFloat(m_percentPos, m_percentValue);
    // End time
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration in seconds (as floating point number)
    std::chrono::duration<float> duration = end - m_startPoint;

    m_spriteShader->setFloat(m_timePos, duration.count());

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectId);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLuint>(1), 0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

}

void GrassRenderer::generateGrassPoints(float density, float mult, float shift) {
    int rez = 1;
    int width = m_heightMap.data().width;
    int height = m_heightMap.data().height;
    unsigned bytePerPixel = m_heightMap.data().nrComponents;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            unsigned char* pixelOffset = m_heightMap.data().dataContent + (j + width * i) * bytePerPixel;
            unsigned char y = pixelOffset[0];

            // Create the vertex
            Prisma::Mesh::Vertex vertex;
            vertex.position.x = -height / 2.0f + height * i / (float)height;
            vertex.position.y = (int)y * mult / 256.0 - shift;
            vertex.position.z = -width / 2.0f + width * j / (float)width;

            // Store the vertex
            m_grassVertices.push_back(vertex);
            // Get the y-value from the vertex array (already scaled and shifted in the original terrain generation)
            glm::mat4 position = glm::translate(glm::mat4(1.0), vertex.position);

            // Combine translation and rotation into the final transformation matrix
            glm::mat4 finalTransform = position;

            glm::mat4 direction = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            // Store the transformation matrix in the m_positions array
            m_positions.push_back({ direction,finalTransform });
        }
    }

    glGenBuffers(1, &m_indirectId);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectId);
    m_command.count = static_cast<GLuint>(m_verticesData.indices.size());
    m_command.instanceCount = m_positions.size();
    m_command.firstIndex = 0;
    m_command.baseVertex = 0;
    m_command.baseInstance = 0;
    glBindBuffer(GL_ARRAY_BUFFER, m_indirectId);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, m_indirectId);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(Prisma::DrawElementsIndirectCommand), &m_command, GL_DYNAMIC_DRAW);

    m_ssbo->resize(sizeof(GrassPosition) * m_positions.size(), GL_STATIC_DRAW);
    m_ssbo->modifyData(0, sizeof(GrassPosition) * m_positions.size(), m_positions.data());

    m_ssboCull->resize(sizeof(GrassPosition) * m_positions.size(), GL_DYNAMIC_READ);
}

void GrassRenderer::projection(glm::mat4 projection) {
    m_projection = projection;
}
