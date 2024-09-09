#include "../../include/Components/TerrainComponent.h"
#include "../../include/Containers/VBO.h"
#include "../../include/Containers/EBO.h"
#include "../../include/GlobalData/GlobalData.h"


Prisma::TerrainComponent::TerrainComponent() : Prisma::Component{}
{
	name("Terrain");
}

void Prisma::TerrainComponent::ui()
{
}

void Prisma::TerrainComponent::updateRender(std::shared_ptr<Prisma::FBO> fbo)
{
}

void Prisma::TerrainComponent::start()
{
	if (m_heightMap) {
        std::vector<float> vertices;
        float yScale = 64.0f / 256.0f, yShift = 16.0f;
        int rez = 1;
        int width = m_heightMap->data().width;
        int height = m_heightMap->data().height;
        unsigned bytePerPixel = m_heightMap->data().nrComponents;
        std::vector<Prisma::Mesh::Vertex> vertexData;

        // Generate vertex data
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                unsigned char* pixelOffset = m_heightMap->dataContent() + (j + width * i) * bytePerPixel;
                unsigned char y = pixelOffset[0];
                Prisma::Mesh::Vertex vertex;
                vertex.position.x = (-height / 2.0f + height * i / (float)height);   // vx
                vertex.position.y = ((int)y * yScale - yShift);   // vy
                vertex.position.z = (-width / 2.0f + width * j / (float)width);   // vz
                vertex.texCoords.x = j / (float)(width - 1);  // u
                vertex.texCoords.y = i / (float)(height - 1); // v
                vertexData.push_back(vertex);
            }
        }
        m_heightMap->freeData();

        std::vector<unsigned int> indices;

        // Generate indices for GL_TRIANGLES
        for (unsigned i = 0; i < height - 1; i += rez)
        {
            for (unsigned j = 0; j < width - 1; j += rez)
            {
                // Get the four corner indices of the quad
                unsigned int bottomLeft = j + width * i;
                unsigned int topLeft = j + width * (i + rez);
                unsigned int bottomRight = (j + rez) + width * i;
                unsigned int topRight = (j + rez) + width * (i + rez);

                // First triangle: bottom-left, top-left, top-right
                indices.push_back(bottomLeft);
                indices.push_back(topLeft);
                indices.push_back(topRight);

                // Second triangle: bottom-left, top-right, bottom-right
                indices.push_back(bottomLeft);
                indices.push_back(topRight);
                indices.push_back(bottomRight);
            }
        }
        
        m_mesh = std::make_shared<Prisma::Mesh>();
        std::shared_ptr<Prisma::Mesh::VerticesData> verticesData = std::make_shared<Prisma::Mesh::VerticesData>();
        verticesData->vertices = vertexData;
        verticesData->indices = indices;
        m_mesh->loadModel(verticesData);
        std::shared_ptr<Prisma::MaterialComponent> materialComponent = std::make_shared<Prisma::MaterialComponent>();
        materialComponent->diffuse({ defaultBlack });
        materialComponent->normal({ defaultNormal });
        materialComponent->roughness_metalness({ defaultBlack });
        m_mesh->material(materialComponent);
        m_mesh->name("TerrainMesh");
        parent()->name("TerrainParent");
        parent()->addChild(m_mesh);
	}
}

void Prisma::TerrainComponent::heightMap(std::shared_ptr<Prisma::Texture> heightMap) {
	m_heightMap = heightMap;
}
