#include "../include/TerrainComponent.h"
#include "../../../Engine/include/Components/Component.h"
#include <glm/glm.hpp>
#include "../../../Engine/include/Helpers/PrismaRender.h"
#include "../../../Engine/include/Components/PhysicsMeshComponent.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

Prisma::TerrainComponent::TerrainComponent() : Prisma::Component{}
{
    name("Terrain");
}

void Prisma::TerrainComponent::ui()
{
    std::vector<ComponentType> components;

    components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "Multiplier", &m_mult));
    components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "Shift", &m_shift));
    components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "Minimum", &m_min));
    components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "Maximum", &m_max));
    components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "Scaling", &m_scale));
    ComponentType componentButton;
    m_startButton = [&]() {
        if (!isStart()) {
            start();
        }
    };
    componentButton = std::make_tuple(Prisma::Component::TYPES::BUTTON, "UI terrain", &m_startButton);

    for (const auto& component : components) {
        addGlobal(component);
    }
    addGlobal(componentButton);
}

void Prisma::TerrainComponent::updateRender(std::shared_ptr<Prisma::FBO> fbo)
{
    glDisable(GL_CULL_FACE);
    m_shader->use();
    m_shader->setMat4(m_modelPos, parent()->finalMatrix());
    m_shader->setInt64(m_heightPos, m_heightMap.id());
    m_shader->setFloat(m_multPos, m_mult);
    m_shader->setFloat(m_shiftPos, m_shift);
    m_shader->setFloat(m_minPos, m_min);
    m_shader->setFloat(m_maxPos, m_max);
    m_shader->setInt64(m_grassPos, m_grass->id());
    m_shader->setInt64(m_stonePos, m_stone->id());
    m_shader->setInt64(m_snowPos, m_snow->id());
    m_shader->setFloat(m_scalePos, m_scale);
    m_shader->setInt64(m_grassNormalPos, m_grassNormal->id());
    m_shader->setInt64(m_stoneNormalPos, m_stoneNormal->id());
    m_shader->setInt64(m_snowNormalPos, m_snowNormal->id());
    m_shader->setInt64(m_grassRoughnessPos, m_grassRoughness->id());
    m_shader->setInt64(m_stoneRoughnessPos, m_stoneRoughness->id());
    m_shader->setInt64(m_snowRoughnessPos, m_snowRoughness->id());

    m_vao.bind();
    glDrawArrays(GL_PATCHES, 0, m_numPatches * m_resolution * m_resolution);

    m_grassRenderer.renderGrass(parent()->finalMatrix());
    glEnable(GL_CULL_FACE);
}

void Prisma::TerrainComponent::generateCpu()
{
    m_grassRenderer.generateGrassPoints(1000,m_mult,m_shift);
    generatePhysics();

    m_heightMap.freeData();
}

void Prisma::TerrainComponent::start()
{
    Prisma::Component::start();
    Prisma::Shader::ShaderHeaders headers;
    m_shader = std::make_shared<Shader>("../../../UserEngine/Shaders/TerrainPipeline/vertex.glsl", "../../../UserEngine/Shaders/TerrainPipeline/fragment.glsl", nullptr, headers, "../../../UserEngine/Shaders/TerrainPipeline/tcsdata.glsl", "../../../UserEngine/Shaders/TerrainPipeline/tesdata.glsl");
    m_csmShader = std::make_shared<Shader>("../../../UserEngine/Shaders/TerrainShadowPipeline/vertex.glsl", "../../../UserEngine/Shaders/TerrainShadowPipeline/fragment.glsl", "../../../UserEngine/Shaders/TerrainShadowPipeline/geometry.glsl", headers, "../../../UserEngine/Shaders/TerrainShadowPipeline/tcsdata.glsl", "../../../UserEngine/Shaders/TerrainShadowPipeline/tesdata.glsl");

    m_grass = std::make_shared<Prisma::Texture>();
    m_stone = std::make_shared<Prisma::Texture>();
    m_snow = std::make_shared<Prisma::Texture>();
    m_grassNormal = std::make_shared<Prisma::Texture>();
    m_stoneNormal = std::make_shared<Prisma::Texture>();
    m_snowNormal = std::make_shared<Prisma::Texture>();
    m_grassRoughness = std::make_shared<Prisma::Texture>();
    m_stoneRoughness = std::make_shared<Prisma::Texture>();
    m_snowRoughness = std::make_shared<Prisma::Texture>();
    m_grass->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/grass.jpg");
    m_stone->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/stone.jpg");
    m_snow->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/snow.jpg");
    m_grassNormal->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/grassNormal.jpg");
    m_stoneNormal->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/stoneNormal.jpg");
    m_snowNormal->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/snowNormal.jpg");
    m_grassRoughness->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/grassRoughness.jpg");
    m_stoneRoughness->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/stoneRoughness.jpg");
    m_snowRoughness->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/snowRoughness.jpg");

    m_shader->use();
    m_modelPos = m_shader->getUniformPosition("model");
    m_heightPos = m_shader->getUniformPosition("heightMap");
    m_multPos = m_shader->getUniformPosition("mult");
    m_shiftPos = m_shader->getUniformPosition("shift");
    m_minPos = m_shader->getUniformPosition("MIN_DISTANCE");
    m_maxPos = m_shader->getUniformPosition("MAX_DISTANCE");
    m_grassPos = m_shader->getUniformPosition("grass");
    m_stonePos = m_shader->getUniformPosition("stone");
    m_snowPos = m_shader->getUniformPosition("snow");
    m_scalePos = m_shader->getUniformPosition("textureScaling");
    m_grassNormalPos = m_shader->getUniformPosition("grassNormal");
    m_stoneNormalPos = m_shader->getUniformPosition("stoneNormal");
    m_snowNormalPos = m_shader->getUniformPosition("snowNormal");
    m_grassRoughnessPos = m_shader->getUniformPosition("grassRoughness");
    m_stoneRoughnessPos = m_shader->getUniformPosition("stoneRoughness");
    m_snowRoughnessPos = m_shader->getUniformPosition("snowRoughness");
    m_grassRenderer.start(m_heightMap);
    generateCpu();
    std::vector<float> vertices;
    int width = m_heightMap.data().width;
    int height = m_heightMap.data().height;
    unsigned bytePerPixel = m_heightMap.data().nrComponents;

    for (unsigned i = 0; i <= m_resolution - 1; i++)
    {
        for (unsigned j = 0; j <= m_resolution - 1; j++)
        {
            vertices.push_back(-width / 2.0f + width * i / (float)m_resolution); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * j / (float)m_resolution); // v.z
            vertices.push_back(i / (float)m_resolution); // u
            vertices.push_back(j / (float)m_resolution); // v

            vertices.push_back(-width / 2.0f + width * (i + 1) / (float)m_resolution); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * j / (float)m_resolution); // v.z
            vertices.push_back((i + 1) / (float)m_resolution); // u
            vertices.push_back(j / (float)m_resolution); // v

            vertices.push_back(-width / 2.0f + width * i / (float)m_resolution); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * (j + 1) / (float)m_resolution); // v.z
            vertices.push_back(i / (float)m_resolution); // u
            vertices.push_back((j + 1) / (float)m_resolution); // v

            vertices.push_back(-width / 2.0f + width * (i + 1) / (float)m_resolution); // v.x
            vertices.push_back(0.0f); // v.y
            vertices.push_back(-height / 2.0f + height * (j + 1) / (float)m_resolution); // v.z
            vertices.push_back((i + 1) / (float)m_resolution); // u
            vertices.push_back((j + 1) / (float)m_resolution); // v
        }
    }

    m_vao.bind();
    Prisma::VBO vbo;
    vbo.writeData(sizeof(float) * vertices.size(), vertices.data());
    m_vao.addAttribPointer(0, 3, 5 * sizeof(float), (void*)0);
    m_vao.addAttribPointer(1, 2, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    glPatchParameteri(GL_PATCH_VERTICES, m_numPatches);
}

void Prisma::TerrainComponent::heightMap(Prisma::Texture heightMap) {
    m_heightMap = heightMap;
}

void Prisma::TerrainComponent::generatePhysics()
{
    int width = m_heightMap.data().width;
    int height = m_heightMap.data().height;
    unsigned bytePerPixel = m_heightMap.data().nrComponents;
    auto mesh = std::make_shared<Prisma::Mesh>();
    mesh->addGlobalList(false);

    auto verticesData = std::make_shared<Prisma::Mesh::VerticesData>();

    unsigned int ratio = 4;

    for (int y = 0; y < height; y = y + ratio) {
        for (int x = 0; x < width; x = x + ratio) {
            int index = (y * width + x) * bytePerPixel;

            // For grayscale image, use the pixel value as the height
            unsigned char pixelValue = m_heightMap.data().dataContent[index];
            float heightValue = static_cast<float>(pixelValue) / 256.0 * m_mult;
            Prisma::Mesh::Vertex v;

            // Create a vertex at (x, heightValue, y)
            v.position = { -width / 2.0f + width * x / (float)width, heightValue, -height / 2.0f + height * y / (float)height };

            // Initialize normal to zero, will be calculated later
            v.normal = { 0.0f, 0.0f, 0.0f };

            verticesData->vertices.push_back(v);
        }
    }

    std::vector<unsigned int> indices;

    int sampledWidth = width / ratio;
    int sampledHeight = height / ratio;

    // Create triangles (2 triangles per pixel quad)
    for (int y = 0; y < sampledHeight - 1; ++y) {
        for (int x = 0; x < sampledWidth - 1; ++x) {
            // Index of the top-left vertex
            unsigned int topLeft = y * sampledWidth + x;
            // Index of the top-right vertex
            unsigned int topRight = topLeft + 1;
            // Index of the bottom-left vertex
            unsigned int bottomLeft = (y + 1) * sampledWidth + x;
            // Index of the bottom-right vertex
            unsigned int bottomRight = bottomLeft + 1;

            // Create two triangles for the current quad (top-left, top-right, bottom-right, bottom-left)
            verticesData->indices.push_back(topLeft);
            verticesData->indices.push_back(bottomLeft);
            verticesData->indices.push_back(bottomRight);
            verticesData->indices.push_back(topLeft);
            verticesData->indices.push_back(bottomRight);
            verticesData->indices.push_back(topRight);

            // Calculate normals for each triangle
            // First triangle (topLeft, bottomLeft, bottomRight)
            glm::vec3 v0 = verticesData->vertices[bottomLeft].position - verticesData->vertices[topLeft].position;
            glm::vec3 v1 = verticesData->vertices[bottomRight].position - verticesData->vertices[topLeft].position;
            glm::vec3 normal1 = glm::normalize(glm::cross(v0, v1));

            // Add normal to each vertex of the first triangle
            verticesData->vertices[topLeft].normal += normal1;
            verticesData->vertices[bottomLeft].normal += normal1;
            verticesData->vertices[bottomRight].normal += normal1;

            // Second triangle (topLeft, bottomRight, topRight)
            glm::vec3 v2 = verticesData->vertices[bottomRight].position - verticesData->vertices[topLeft].position;
            glm::vec3 v3 = verticesData->vertices[topRight].position - verticesData->vertices[topLeft].position;
            glm::vec3 normal2 = glm::normalize(glm::cross(v2, v3));

            // Add normal to each vertex of the second triangle
            verticesData->vertices[topLeft].normal += normal2;
            verticesData->vertices[bottomRight].normal += normal2;
            verticesData->vertices[topRight].normal += normal2;
        }
    }

    // Normalize all the normals (convert accumulated normals to unit length)
    for (auto& vertex : verticesData->vertices) {
        vertex.normal = glm::normalize(vertex.normal);
    }

    mesh->loadModel(verticesData);
    auto physicsComponent = std::make_shared<Prisma::PhysicsMeshComponent>();
    physicsComponent->collisionData({ Prisma::Physics::Collider::LANDSCAPE_COLLIDER,0.0,btVector3(0.0,0.0,0.0),true });

    mesh->name("TerrainMesh");
    mesh->addComponent(physicsComponent);
    parent()->addChild(mesh);
}