#include "../../include/Components/TerrainComponent.h"
#include "../../include/Containers/VBO.h"
#include "../../include/Containers/EBO.h"


Prisma::TerrainComponent::TerrainComponent() : Prisma::Component{}
{
	name("Terrain");
}

void Prisma::TerrainComponent::ui()
{
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

    m_vao.bind();
    glDrawArrays(GL_PATCHES, 0, m_numPatches * m_resolution * m_resolution);
    glEnable(GL_CULL_FACE);
}

void Prisma::TerrainComponent::start()
{
	if (m_heightMap) {
        Prisma::Shader::ShaderHeaders headers;
        m_shader = std::make_shared<Shader>("../../../Engine/Shaders/TerrainPipeline/vertex.glsl", "../../../Engine/Shaders/TerrainPipeline/fragment.glsl",nullptr, headers, "../../../Engine/Shaders/TerrainPipeline/tcsdata.glsl", "../../../Engine/Shaders/TerrainPipeline/tesdata.glsl");
        m_grass = std::make_shared<Prisma::Texture>();
        m_stone = std::make_shared<Prisma::Texture>();
        m_snow = std::make_shared<Prisma::Texture>();
        m_grassNormal = std::make_shared<Prisma::Texture>();
        m_stoneNormal = std::make_shared<Prisma::Texture>();
        m_snowNormal = std::make_shared<Prisma::Texture>();
        m_grass->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/grass.jpg");
        m_stone->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/stone.jpg");
        m_snow->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/snow.jpg");
        m_grassNormal->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/grassNormal.jpg");
        m_stoneNormal->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/stoneNormal.jpg");
        m_snowNormal->loadTexture("../../../Resources/DefaultScene/Heightmaps/Levels/snowNormal.jpg");

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

        GLint maxTessLevel;
        glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);
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
