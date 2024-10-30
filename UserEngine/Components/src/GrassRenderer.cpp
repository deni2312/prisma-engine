#include "../include/GrassRenderer.h"
#include <random>
#include <glm/gtx/string_cast.hpp>

void GrassRenderer::start(Prisma::Texture heightMap)
{
	m_heightMap = heightMap;
	m_spriteShader = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/GrassPipeline/vertex.glsl",
	                                                  "../../../UserEngine/Shaders/GrassPipeline/fragment.glsl");
	m_cullShader = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/GrassPipeline/compute.glsl");
	m_grassSprite = std::make_shared<Prisma::Texture>();
	m_grassSprite->loadTexture({"../../../Resources/DefaultScene/sprites/grass.png", false, true, false});
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

	auto grass = sceneLoader.loadScene("../../../Resources/DefaultScene/grass/grass.gltf", {true});
	m_grassMesh = std::dynamic_pointer_cast<Prisma::Mesh>(grass->root->children()[0]);
	m_verticesData = m_grassMesh->verticesData();

	float yMax = m_verticesData.vertices[0].position.y;

	for (const auto& v : m_verticesData.vertices)
	{
		if (v.position.y > yMax)
		{
			yMax = v.position.y;
		}
	}
	m_percentValue = yMax / 100.0;

	m_vao.bind();
	Prisma::VBO vbo;

	Prisma::EBO ebo;

	vbo.writeData(m_verticesData.vertices.size() * sizeof(Prisma::Mesh::Vertex), &m_verticesData.vertices[0],
	              GL_DYNAMIC_DRAW);
	ebo.writeData(m_verticesData.indices.size() * sizeof(unsigned int), &m_verticesData.indices[0], GL_DYNAMIC_DRAW);

	m_vao.addAttribPointer(0, 3, sizeof(Prisma::Mesh::Vertex), nullptr);
	m_vao.addAttribPointer(1, 3, sizeof(Prisma::Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, normal));
	m_vao.addAttribPointer(2, 2, sizeof(Prisma::Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, texCoords));
	m_vao.resetVao();
}

void GrassRenderer::renderGrass(glm::mat4 translation)
{
	m_cullShader->use();
	m_cullShader->setMat4(m_modelComputePos, translation);
	m_cullShader->setMat4(m_projectionPos, m_projection);
	unsigned int sizeCluster = 8;
	unsigned int sizePositions = glm::ceil(glm::sqrt(m_grassPositions.size() / (sizeCluster * sizeCluster)));
	m_cullShader->dispatchCompute({sizePositions, sizePositions, 1});
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, m_indirectId);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLuint>(1), 0);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void GrassRenderer::generateGrassPoints(float density, float mult, float shift)
{
	generateGrassPositions(density, mult, shift);
	glGenBuffers(1, &m_indirectId);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectId);
	m_command.count = static_cast<GLuint>(m_verticesData.indices.size());
	m_command.instanceCount = m_grassPositions.size();
	m_command.firstIndex = 0;
	m_command.baseVertex = 0;
	m_command.baseInstance = 0;
	glBindBuffer(GL_ARRAY_BUFFER, m_indirectId);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 17, m_indirectId);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(Prisma::DrawElementsIndirectCommand), &m_command, GL_DYNAMIC_DRAW);

	m_ssbo->resize(sizeof(GrassPosition) * m_grassPositions.size(), GL_STATIC_DRAW);
	m_ssbo->modifyData(0, sizeof(GrassPosition) * m_grassPositions.size(), m_grassPositions.data());

	m_ssboCull->resize(sizeof(GrassPosition) * m_grassPositions.size(), GL_DYNAMIC_READ);
}

void GrassRenderer::projection(glm::mat4 projection)
{
	m_projection = projection;
}

void GrassRenderer::generateGrassPositions(float density, float mult, float shift)
{
	// Seed the random number generator once
	std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));

	// Create a uniform distribution for small random displacements
	std::uniform_real_distribution<float> displacementDist(-0.05f, 0.05f); // Random displacement in range [-0.05, 0.05]
	std::uniform_real_distribution<float> scaleDist(0.5f, 1.0f); // Random scale in range [0.1, 1.0]
	std::uniform_real_distribution<float> rotationDist(0.0f, glm::two_pi<float>()); // Random rotation between [0, 2π]

	// Assuming glm::two_pi<float>() provides 2*π radians, equivalent to 360 degrees.

	int width = m_heightMap.data().width;
	int height = m_heightMap.data().height;
	unsigned bytePerPixel = m_heightMap.data().nrComponents;

	for (int i = 0; i < height * density; i++)
	{
		for (int j = 0; j < width * density; j++)
		{
			// Find the coordinates in the original heightmap space
			float original_i = i / density;
			float original_j = j / density;

			// Add random displacement to the original coordinates
			float displaced_i = original_i + displacementDist(rng);
			float displaced_j = original_j + displacementDist(rng);

			// Ensure the displaced coordinates remain within bounds
			displaced_i = std::clamp(displaced_i, 0.0f, static_cast<float>(height - 1));
			displaced_j = std::clamp(displaced_j, 0.0f, static_cast<float>(width - 1));

			// Get the indices of the four neighboring heightmap vertices for interpolation
			int x0 = static_cast<int>(displaced_j);
			int x1 = std::min(x0 + 1, width - 1); // Clamp to width
			int z0 = static_cast<int>(displaced_i);
			int z1 = std::min(z0 + 1, height - 1); // Clamp to height

			// Compute interpolation weights
			float tx = displaced_j - x0;
			float tz = displaced_i - z0;

			// Get the heightmap heights for the four neighbors
			unsigned char* pixel00 = m_heightMap.data().dataContent + (x0 + width * z0) * bytePerPixel;
			unsigned char* pixel01 = m_heightMap.data().dataContent + (x1 + width * z0) * bytePerPixel;
			unsigned char* pixel10 = m_heightMap.data().dataContent + (x0 + width * z1) * bytePerPixel;
			unsigned char* pixel11 = m_heightMap.data().dataContent + (x1 + width * z1) * bytePerPixel;

			// Extract height values
			float h00 = static_cast<int>(pixel00[0]) * mult / 256.0f - shift;
			float h01 = static_cast<int>(pixel01[0]) * mult / 256.0f - shift;
			float h10 = static_cast<int>(pixel10[0]) * mult / 256.0f - shift;
			float h11 = static_cast<int>(pixel11[0]) * mult / 256.0f - shift;

			// Bilinear interpolation of the height
			float interpolatedHeight = (1 - tx) * (1 - tz) * h00 +
				tx * (1 - tz) * h01 +
				(1 - tx) * tz * h10 +
				tx * tz * h11;

			// Create the vertex with random displacement
			Prisma::Mesh::Vertex vertex;
			vertex.position.x = -height / 2.0f + height * displaced_i / static_cast<float>(height);
			vertex.position.y = interpolatedHeight;
			vertex.position.z = -width / 2.0f + width * displaced_j / static_cast<float>(width);

			// Store the vertex
			m_grassVertices.push_back(vertex);

			// Random scale for the grass blade
			float randomScale = scaleDist(rng);
			glm::mat4 scaleMatrix = scale(glm::mat4(1.0f), glm::vec3(randomScale));

			// Random rotation around the Y-axis (up)
			float randomRotation = rotationDist(rng);
			glm::mat4 rotationMatrix = rotate(glm::mat4(1.0f), randomRotation, glm::vec3(0.0f, 1.0f, 0.0f)) *
				scaleMatrix;

			// Set the position matrix
			glm::mat4 positionMatrix = translate(glm::mat4(1.0f), vertex.position);

			// Combine transformations: scale -> rotate -> translate
			glm::mat4 finalTransform = positionMatrix;

			// Store the transformation matrix in the m_positions array
			m_grassPositions.push_back({rotationMatrix, finalTransform});
		}
	}
}
