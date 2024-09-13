#include "../../include/Components/TerrainComponent.h"
#include "../../include/Containers/VBO.h"
#include "../../include/Containers/EBO.h"
#include "../../include/SceneObjects/Sprite.h"
#include "../../include/Helpers/IBLBuilder.h"

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
    m_shader->setInt64(m_heightPos, m_heightMap->id());
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

    m_spriteShader->use();
    m_spriteShader->setInt64(m_spritePos, m_grassSprite->id());
    m_spriteShader->setMat4(m_spriteModelPos, m_spriteModel);

    Prisma::IBLBuilder::getInstance().renderQuad(m_positions.size());
    m_spriteShader->setMat4(m_spriteModelPos, m_spriteModelRotation);

    Prisma::IBLBuilder::getInstance().renderQuad(m_positions.size());
    glEnable(GL_CULL_FACE);

}

void Prisma::TerrainComponent::generateCpu()
{
    int width = m_heightMap->data().width;
    int height = m_heightMap->data().height;
    unsigned bytePerPixel = m_heightMap->data().nrComponents;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            unsigned char* pixelOffset = m_heightMap->dataContent() + (j + width * i) * bytePerPixel;
            unsigned char y = pixelOffset[0];

            // vertex
            m_grassVertices.push_back(-height / 2.0f + height * i / (float)height);   // vx
            m_grassVertices.push_back((int)y * m_mult - m_shift);   // vy
            m_grassVertices.push_back(-width / 2.0f + width * j / (float)width);   // vz
        }
    }
    m_heightMap->freeData();
    generateGrassPoints(0.1);
    m_ssbo = std::make_shared<Prisma::SSBO>(15);
    m_ssbo->resize(sizeof(glm::mat4) * m_positions.size());
    m_ssbo->modifyData(0, sizeof(glm::mat4) * m_positions.size(), m_positions.data());
}

void Prisma::TerrainComponent::start()
{
    Prisma::Component::start();
	if (m_heightMap) {
        Prisma::Shader::ShaderHeaders headers;
        m_shader = std::make_shared<Shader>("../../../Engine/Shaders/TerrainPipeline/vertex.glsl", "../../../Engine/Shaders/TerrainPipeline/fragment.glsl",nullptr, headers, "../../../Engine/Shaders/TerrainPipeline/tcsdata.glsl", "../../../Engine/Shaders/TerrainPipeline/tesdata.glsl");
        m_csmShader = std::make_shared<Shader>("../../../Engine/Shaders/TerrainShadowPipeline/vertex.glsl", "../../../Engine/Shaders/TerrainShadowPipeline/fragment.glsl", "../../../Engine/Shaders/TerrainShadowPipeline/geometry.glsl", headers, "../../../Engine/Shaders/TerrainShadowPipeline/tcsdata.glsl", "../../../Engine/Shaders/TerrainShadowPipeline/tesdata.glsl");
        m_spriteShader = std::make_shared<Shader>("../../../Engine/Shaders/GrassPipeline/vertex.glsl", "../../../Engine/Shaders/GrassPipeline/fragment.glsl");


        m_grass = std::make_shared<Prisma::Texture>();
        m_stone = std::make_shared<Prisma::Texture>();
        m_snow = std::make_shared<Prisma::Texture>();
        m_grassNormal = std::make_shared<Prisma::Texture>();
        m_stoneNormal = std::make_shared<Prisma::Texture>();
        m_snowNormal = std::make_shared<Prisma::Texture>();
        m_grassRoughness = std::make_shared<Prisma::Texture>();
        m_stoneRoughness = std::make_shared<Prisma::Texture>();
        m_snowRoughness = std::make_shared<Prisma::Texture>();
        m_grassSprite = std::make_shared<Prisma::Texture>();
        m_grass->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/grass.jpg");
        m_stone->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/stone.jpg");
        m_snow->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/snow.jpg");
        m_grassNormal->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/grassNormal.jpg");
        m_stoneNormal->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/stoneNormal.jpg");
        m_snowNormal->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/snowNormal.jpg");
        m_grassRoughness->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/grassRoughness.jpg");
        m_stoneRoughness->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/stoneRoughness.jpg");
        m_snowRoughness->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/snowRoughness.jpg");
        m_grassSprite->loadTexture("../../../Resources/DefaultScene/sprites/grass.png");

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
        m_spriteShader->use();
        m_spritePos = m_spriteShader->getUniformPosition("grassSprite");
        m_spriteModelPos = m_spriteShader->getUniformPosition("model");
        m_spriteModel = glm::rotate(glm::mat4(1.0), glm::radians(180.0f), glm::vec3(0, 0, 1));
        m_spriteModelRotation = glm::rotate(glm::mat4(1.0), glm::radians(180.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::mat4(1.0), glm::radians(90.0f), glm::vec3(0, 1, 0));
        generateCpu();
        std::vector<float> vertices;
        int width = m_heightMap->data().width;
        int height = m_heightMap->data().height;
        unsigned bytePerPixel = m_heightMap->data().nrComponents;

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
}

void Prisma::TerrainComponent::heightMap(std::shared_ptr<Prisma::Texture> heightMap) {
	m_heightMap = heightMap;
}

void Prisma::TerrainComponent::generateGrassPoints(float density)
{
    int width = m_heightMap->data().width;
    int height = m_heightMap->data().height;
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
        int vertexIndex = 3 * (gridZ * width + gridX);
        // Get the y-value from the vertex array (already scaled and shifted in the original terrain generation)
        float y = m_grassVertices[vertexIndex + 1];

        glm::vec4 point(x, (y-m_shift)/(m_mult*m_heightMap->data().nrComponents)+1, z, 1.0);
        glm::mat4 pointData(1.0);
        pointData[3] = point;
        m_positions.push_back(pointData);
    }
}
