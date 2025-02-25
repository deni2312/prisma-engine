#include "../include/TerrainComponent.h"
#include "../../../Engine/include/Components/Component.h"
#include <glm/glm.hpp>
#include "../../../Engine/include/Helpers/PrismaRender.h"
#include "../../../Engine/include/Components/PhysicsMeshComponent.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include "../../../Engine/include/Helpers/SettingsLoader.h"

Prisma::TerrainComponent::TerrainComponent() : Component{}
{
	name("Terrain");
}

void Prisma::TerrainComponent::ui()
{
	std::vector<ComponentType> components;

	components.push_back(std::make_tuple(TYPES::FLOAT, "Multiplier", &m_mult));
	components.push_back(std::make_tuple(TYPES::FLOAT, "Shift", &m_shift));
	components.push_back(std::make_tuple(TYPES::FLOAT, "Scaling", &m_scale));
	components.push_back(std::make_tuple(TYPES::FLOAT, "Far plane", &m_farPlane));
	ComponentType componentButton;
	m_startButton = [&]()
	{
		if (!isStart())
		{
			start();
		}
	};
	componentButton = std::make_tuple(TYPES::BUTTON, "UI terrain", &m_startButton);

	ComponentType componentApply;
	m_apply = [&]()
	{
		if (isStart())
		{
			auto settings = SettingsLoader::getInstance().getSettings();
			m_grassRenderer.projection(glm::perspective(
				glm::radians(Prisma::GlobalData::getInstance().currentGlobalScene()->camera->angle()),
				static_cast<float>(settings.width) / static_cast<float>(settings
					.height), Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane(),
				m_farPlane));
		}
	};
	componentApply = std::make_tuple(TYPES::BUTTON, "Apply", &m_apply);

	for (const auto& component : components)
	{
		addGlobal({component,false });
	}
	addGlobal({componentButton,false });
	addGlobal({componentApply,false });
}

void Prisma::TerrainComponent::updateRender(std::shared_ptr<FBO> fbo)
{
	glDisable(GL_CULL_FACE);
	m_shader->use();
	m_shader->setMat4(m_modelPos, parent()->finalMatrix());
	m_shader->setInt64(m_heightPos, m_heightMap.id());
	m_shader->setFloat(m_multPos, m_mult);
	m_shader->setFloat(m_shiftPos, m_shift);
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
	glDrawElements(GL_TRIANGLES, m_vertices->indices.size(), GL_UNSIGNED_INT, 0);
	m_grassRenderer.renderGrass(parent()->finalMatrix());

	glEnable(GL_CULL_FACE);
}

void Prisma::TerrainComponent::generateCpu()
{
	m_grassRenderer.generateGrassPoints(8, m_mult, m_shift);
	generatePhysics();
}

void Prisma::TerrainComponent::start()
{
	Component::start();
	m_vertices = std::make_shared<Prisma::Mesh::VerticesData>();
	Shader::ShaderHeaders headers;
	m_shader = std::make_shared<Shader>("../../../UserEngine/Shaders/TerrainPipeline/vertex.glsl",
	                                    "../../../UserEngine/Shaders/TerrainPipeline/fragment.glsl");
	m_csmShader = std::make_shared<Shader>("../../../UserEngine/Shaders/TerrainShadowPipeline/vertex.glsl",
	                                       "../../../UserEngine/Shaders/TerrainShadowPipeline/fragment.glsl",
	                                       "../../../UserEngine/Shaders/TerrainShadowPipeline/geometry.glsl", headers,
	                                       "../../../UserEngine/Shaders/TerrainShadowPipeline/tcsdata.glsl",
	                                       "../../../UserEngine/Shaders/TerrainShadowPipeline/tesdata.glsl");

	m_grass = std::make_shared<Texture>();
	m_stone = std::make_shared<Texture>();
	m_snow = std::make_shared<Texture>();
	m_grassNormal = std::make_shared<Texture>();
	m_stoneNormal = std::make_shared<Texture>();
	m_snowNormal = std::make_shared<Texture>();
	m_grassRoughness = std::make_shared<Texture>();
	m_stoneRoughness = std::make_shared<Texture>();
	m_snowRoughness = std::make_shared<Texture>();
	m_grass->loadTexture({"../../../Resources/DefaultScene/Heightmaps/Levels/grass.jpg"});
	m_stone->loadTexture({"../../../Resources/DefaultScene/Heightmaps/Levels/stone.jpg"});
	m_snow->loadTexture({"../../../Resources/DefaultScene/Heightmaps/Levels/snow.jpg"});
	m_grassNormal->loadTexture({"../../../Resources/DefaultScene/Heightmaps/Levels/grassNormal.jpg"});
	m_stoneNormal->loadTexture({"../../../Resources/DefaultScene/Heightmaps/Levels/stoneNormal.jpg"});
	m_snowNormal->loadTexture({"../../../Resources/DefaultScene/Heightmaps/Levels/snowNormal.jpg"});
	m_grassRoughness->loadTexture({"../../../Resources/DefaultScene/Heightmaps/Levels/grassRoughness.jpg"});
	m_stoneRoughness->loadTexture({"../../../Resources/DefaultScene/Heightmaps/Levels/stoneRoughness.jpg"});
	m_snowRoughness->loadTexture({"../../../Resources/DefaultScene/Heightmaps/Levels/snowRoughness.jpg"});

	m_shader->use();
	m_modelPos = m_shader->getUniformPosition("model");
	m_heightPos = m_shader->getUniformPosition("heightMap");
	m_multPos = m_shader->getUniformPosition("mult");
	m_shiftPos = m_shader->getUniformPosition("shift");
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
	m_farPlane = Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane();
	auto settings = SettingsLoader::getInstance().getSettings();
	m_grassRenderer.projection(glm::perspective(
		glm::radians(Prisma::GlobalData::getInstance().currentGlobalScene()->camera->angle()),
		static_cast<float>(settings.width) / static_cast<float>(settings.
			height), Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane(), m_farPlane));
	generateCpu();
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
			Mesh::Vertex v;
			v.position.x = -height / 2.0f + height * i / static_cast<float>(height);
			v.position.y = static_cast<int>(y) * m_mult / 256.0 - m_shift;
			v.position.z = -width / 2.0f + width * j / static_cast<float>(width);
			v.texCoords.x = j / static_cast<float>(width - 1); // Normalized texture coordinate (0 to 1)
			v.texCoords.y = i / static_cast<float>(height - 1); // Normalized texture coordinate (0 to 1)

			m_vertices->vertices.push_back(v);
		}
	}

	// Calculate normals
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			glm::vec3 left, right, up, down;
			glm::vec3 normal(0.0f, 0.0f, 0.0f);

			// Get current vertex
			Mesh::Vertex& v = m_vertices->vertices[i * width + j];

			// Calculate vectors around the current vertex
			if (j > 0) // Left
				left = m_vertices->vertices[i * width + (j - 1)].position - v.position;
			if (j < width - 1) // Right
				right = m_vertices->vertices[i * width + (j + 1)].position - v.position;
			if (i > 0) // Up
				up = m_vertices->vertices[(i - 1) * width + j].position - v.position;
			if (i < height - 1) // Down
				down = m_vertices->vertices[(i + 1) * width + j].position - v.position;

			// Calculate normals using cross products of adjacent vectors
			if (j > 0 && i < height - 1) // Bottom-left
				normal += normalize(cross(down, left));
			if (j < width - 1 && i < height - 1) // Bottom-right
				normal += normalize(cross(right, down));
			if (j < width - 1 && i > 0) // Top-right
				normal += normalize(cross(up, right));
			if (j > 0 && i > 0) // Top-left
				normal += normalize(cross(left, up));

			v.normal = normalize(normal); // Normalize the accumulated normal
		}
	}

	for (unsigned i = 0; i < height - 1; i++)
	{
		for (unsigned j = 0; j < width - 1; j++)
		{
			unsigned int topLeft = i * width + j;
			unsigned int topRight = topLeft + 1;
			unsigned int bottomLeft = (i + 1) * width + j;
			unsigned int bottomRight = bottomLeft + 1;

			// First triangle
			m_vertices->indices.push_back(topLeft);
			m_vertices->indices.push_back(bottomLeft);
			m_vertices->indices.push_back(topRight);

			// Second triangle
			m_vertices->indices.push_back(topRight);
			m_vertices->indices.push_back(bottomLeft);
			m_vertices->indices.push_back(bottomRight);
		}
	}

	m_strips = (height - 1) / rez;
	m_stripTris = (width / rez) * 2 - 2;

	m_vao.bind();
	VBO vbo;
	EBO ebo;
	vbo.writeData(m_vertices->vertices.size() * sizeof(Mesh::Vertex), &m_vertices->vertices[0]);
	ebo.writeData(sizeof(unsigned int) * m_vertices->indices.size(), m_vertices->indices.data());
	// link vertex attributes
	m_vao.addAttribPointer(0, 3, sizeof(Mesh::Vertex), nullptr);
	m_vao.addAttribPointer(1, 3, sizeof(Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, normal));
	m_vao.addAttribPointer(2, 2, sizeof(Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, texCoords));
	m_mesh->loadModel(m_vertices);
}

void Prisma::TerrainComponent::heightMap(Texture heightMap)
{
	m_heightMap = heightMap;
}

void Prisma::TerrainComponent::generatePhysics()
{
	int width = m_heightMap.data().width;
	int height = m_heightMap.data().height;
	unsigned bytePerPixel = m_heightMap.data().nrComponents;
	m_mesh = std::make_shared<Mesh>();
	m_mesh->addGlobalList(false);

	unsigned int ratio = 1;

	Physics::LandscapeData landscapeData;
	landscapeData.offset = Vec3(-width / 2, 0, -width / 2);
	landscapeData.scale = Vec3(1, m_mult, 1);
	landscapeData.width = width;

	for (int y = 0; y < height; y = y + ratio)
	{
		for (int x = 0; x < width; x = x + ratio)
		{
			int index = (x * width + y) * bytePerPixel;

			// For grayscale image, use the pixel value as the height
			unsigned char pixelValue = m_heightMap.data().dataContent[index];
			landscapeData.landscape.push_back(static_cast<float>(pixelValue) / 256.0);
		}
	}

	auto physicsComponent = std::make_shared<PhysicsMeshComponent>();

	physicsComponent->landscapeData(landscapeData);
	physicsComponent->collisionData({Physics::Collider::LANDSCAPE_COLLIDER, 0.0, false});
	m_mesh->name("TerrainMesh");
	m_mesh->addComponent(physicsComponent);
	parent()->addChild(m_mesh);
}
