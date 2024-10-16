#include "../include/GrassRenderer.h"

void GrassRenderer::start(Prisma::Texture heightMap) {
    m_heightMap = heightMap;
    m_spriteShader = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/GrassPipeline/vertex.glsl", "../../../UserEngine/Shaders/GrassPipeline/fragment.glsl");
    m_cullShader = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/GrassPipeline/compute.glsl");
    m_grassSprite = std::make_shared<Prisma::Texture>();
    m_grassSprite->loadTexture("../../../Resources/DefaultScene/sprites/grass.png", false, true, false);
    m_spriteShader->use();
    m_spritePos = m_spriteShader->getUniformPosition("grassSprite");
    m_spriteModelPos = m_spriteShader->getUniformPosition("model");
    for (int i = 0; i < 4; i++) {
        m_spriteModelRotation.push_back(glm::rotate(glm::mat4(1.0), glm::radians(45.0f * i), glm::vec3(0, 1, 0)));
    }
    m_cullShader->use();
    m_modelComputePos = m_cullShader->getUniformPosition("model");
    m_ssbo = std::make_shared<Prisma::SSBO>(15);
    m_ssboCull = std::make_shared<Prisma::SSBO>(16);

    Prisma::SceneLoader sceneLoader;

    auto grass = sceneLoader.loadScene("../../../Resources/DefaultScene/grass/grass.gltf", { true });
    m_grassMesh = std::dynamic_pointer_cast<Prisma::Mesh>(grass->root->children()[0]);
    m_verticesData = m_grassMesh->verticesData();
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
    unsigned int sizeCluster = 8;
    unsigned int sizePositions = glm::ceil(glm::sqrt(m_positions.size() / (sizeCluster * sizeCluster)));
    m_cullShader->dispatchCompute({ sizePositions,sizePositions,1 });
    m_cullShader->wait(GL_SHADER_STORAGE_BARRIER_BIT);
    m_vao.bind();
    m_spriteShader->use();
    m_spriteShader->setInt64(m_spritePos, m_grassSprite->id());
    m_spriteShader->setMat4(m_spriteModelPos, translation * m_grassMesh->finalMatrix());

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectId);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLuint>(1), 0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

}

void GrassRenderer::generateGrassPoints(float density, float mult, float shift) {
    int width = m_heightMap.data().width;
    int height = m_heightMap.data().height;
    unsigned bytePerPixel = m_heightMap.data().nrComponents;

    auto getHeightAt = [&](int x, int z) -> float {
        if (x < 0 || x >= width || z < 0 || z >= height)
            return 0.0f;  // Handle out-of-bound cases
        unsigned char* pixelOffset = m_heightMap.data().dataContent + (z + width * x) * bytePerPixel;
        unsigned char y = pixelOffset[0];
        return (float)(y * mult - shift) / 256.0;
    };

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            // Get the height of the current vertex and its neighbors
            float heightL = getHeightAt(i - 1, j);  // Left
            float heightR = getHeightAt(i + 1, j);  // Right
            float heightD = getHeightAt(i, j - 1);  // Down
            float heightU = getHeightAt(i, j + 1);  // Up
            glm::vec3 tangent = glm::vec3(2.0, heightR - heightL, 0.0);
            glm::vec3 bitangent = glm::vec3(0.0, heightD - heightU, 2.0);

            // Calculate the normal using the formula
            glm::vec3 normal = glm::cross(tangent, bitangent);

            // Create the vertex
            Prisma::Mesh::Vertex vertex;
            vertex.position.x = -height / 2.0f + height * i / (float)height;
            vertex.position.y = getHeightAt(i, j);
            vertex.position.z = -width / 2.0f + width * j / (float)width;
            vertex.normal = normal;  // Assign the calculated normal to the vertex

            // Store the vertex
            m_grassVertices.push_back(vertex);
        }
    }
    // Set seed for random number generation
    srand(static_cast<unsigned int>(time(0)));

    // The density determines how many grass points to generate
    int numPoints = static_cast<int>(density * width * height);
    int vertexStride = 3;


    for (int i = 0; i < numPoints; ++i)
    {
        // Generate random x, z positions within the terrain range
        float x = -width / 2.0f + (rand() / (float)RAND_MAX) * width;
        float z = -height / 2.0f + (rand() / (float)RAND_MAX) * height;

        // Find the corresponding vertex position on the terrain
        int gridX = static_cast<int>((x + width / 2.0f) / width * width);
        int gridZ = static_cast<int>((z + height / 2.0f) / height * height);

        // Ensure we are within the bounds
        if (gridX >= width) gridX = width - 1;
        if (gridZ >= height) gridZ = height - 1;

        // Get the index of the corresponding vertex
        int vertexIndex = gridZ * width + gridX;
        // Get the y-value from the vertex array (already scaled and shifted in the original terrain generation)
        float y = m_grassVertices[vertexIndex].position.y;
        auto normal = glm::normalize(m_grassVertices[vertexIndex].normal);
        m_positions.push_back(glm::vec4(x, y, z, 1.0));
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

    m_ssbo->resize(sizeof(glm::vec4) * m_positions.size(), GL_STATIC_DRAW);
    m_ssbo->modifyData(0, sizeof(glm::vec4) * m_positions.size(), m_positions.data());

    m_ssboCull->resize(sizeof(glm::vec4) * m_positions.size(), GL_DYNAMIC_READ);
}