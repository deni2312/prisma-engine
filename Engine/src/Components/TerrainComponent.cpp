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
    m_vao.bind();
    for (unsigned strip = 0; strip < m_numStrips; strip++)
    {
        glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
            m_numTrisPerStrip + 2,   // number of indices to render
            GL_UNSIGNED_INT,     // index data type
            (void*)(sizeof(unsigned) * (m_numTrisPerStrip + 2) * strip)); // offset to starting index
    }
    glEnable(GL_CULL_FACE);
}

void Prisma::TerrainComponent::start()
{
	if (m_heightMap) {
        m_shader = std::make_shared<Shader>("../../../Engine/Shaders/TerrainPipeline/vertex.glsl", "../../../Engine/Shaders/TerrainPipeline/fragment.glsl");
        m_shader->use();
        m_modelPos = m_shader->getUniformPosition("model");
        std::vector<float> vertices;
        float yScale = 64.0f / 256.0f, yShift = 16.0f;
        int rez = 1;
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
                vertices.push_back(-height / 2.0f + height * i / (float)height);   // vx
                vertices.push_back((int)y * yScale - yShift);   // vy
                vertices.push_back(-width / 2.0f + width * j / (float)width);   // vz
            }
        }
        m_heightMap->freeData();

        std::vector<unsigned int> indices;
        for (unsigned i = 0; i < height - 1; i += rez)
        {
            for (unsigned j = 0; j < width; j += rez)
            {
                for (unsigned k = 0; k < 2; k++)
                {
                    indices.push_back(j + width * (i + k * rez));
                }
            }
        }

        m_numStrips = (height - 1) / rez;
        m_numTrisPerStrip = (width / rez) * 2 - 2;

        m_vao.bind();
        Prisma::VBO vbo;
        Prisma::EBO ebo;
        vbo.writeData(sizeof(float)*vertices.size(), vertices.data());
        ebo.writeData(sizeof(unsigned int)*indices.size(), indices.data());
        // link vertex attributes
        m_vao.addAttribPointer(0, 3, 3 * sizeof(float), (void*)0);
	}
}

void Prisma::TerrainComponent::heightMap(std::shared_ptr<Prisma::Texture> heightMap) {
	m_heightMap = heightMap;
}
