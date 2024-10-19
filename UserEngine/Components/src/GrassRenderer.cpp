#include "../include/GrassRenderer.h"
#include <random>
#include <glm/gtx/string_cast.hpp>

void GrassRenderer::start(Prisma::Texture heightMap) {
    m_heightMap = heightMap;
    m_spriteShader = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/GrassPipeline/vertex.glsl", "../../../UserEngine/Shaders/GrassPipeline/fragment.glsl");
    m_cullShader = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/GrassPipeline/compute.glsl");
    m_grassSprite = std::make_shared<Prisma::Texture>();
    m_grassSprite->loadTexture({ "../../../Resources/DefaultScene/sprites/grass.png", false, true, false });
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
    int width = m_heightMap.data().width;
    int height = m_heightMap.data().height;
    unsigned bytePerPixel = m_heightMap.data().nrComponents;

    float densityFactor = 8.0f; // Increase this to make grass placement denser

    for (int i = 0; i < height * densityFactor; i++)
    {
        for (int j = 0; j < width * densityFactor; j++)
        {
            // Find the coordinates in the original heightmap space
            float original_i = i / densityFactor;
            float original_j = j / densityFactor;

            // Get the indices of the four neighboring heightmap vertices for interpolation
            int x0 = static_cast<int>(original_j);
            int x1 = std::min(x0 + 1, width - 1); // clamp to width
            int z0 = static_cast<int>(original_i);
            int z1 = std::min(z0 + 1, height - 1); // clamp to height

            // Compute interpolation weights
            float tx = original_j - x0;
            float tz = original_i - z0;

            // Get the heightmap heights for the four neighbors
            unsigned char* pixel00 = m_heightMap.data().dataContent + (x0 + width * z0) * bytePerPixel;
            unsigned char* pixel01 = m_heightMap.data().dataContent + (x1 + width * z0) * bytePerPixel;
            unsigned char* pixel10 = m_heightMap.data().dataContent + (x0 + width * z1) * bytePerPixel;
            unsigned char* pixel11 = m_heightMap.data().dataContent + (x1 + width * z1) * bytePerPixel;

            // Extract height values
            float h00 = (int)pixel00[0] * mult / 256.0f - shift;
            float h01 = (int)pixel01[0] * mult / 256.0f - shift;
            float h10 = (int)pixel10[0] * mult / 256.0f - shift;
            float h11 = (int)pixel11[0] * mult / 256.0f - shift;

            // Bilinear interpolation of the height
            float interpolatedHeight = (1 - tx) * (1 - tz) * h00 +
                tx * (1 - tz) * h01 +
                (1 - tx) * tz * h10 +
                tx * tz * h11;

            // Create the vertex
            Prisma::Mesh::Vertex vertex;
            vertex.position.x = -height / 2.0f + height * original_i / (float)height;
            vertex.position.y = interpolatedHeight;
            vertex.position.z = -width / 2.0f + width * original_j / (float)width;

            // Store the vertex
            m_grassVertices.push_back(vertex);

            // Set the transformation matrix for the grass
            glm::mat4 position = glm::translate(glm::mat4(1.0f), vertex.position);
            glm::mat4 finalTransform = position;

            glm::mat4 direction = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));

            // Store the transformation matrix in the m_positions array
            m_positions.push_back({ direction, finalTransform });
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
