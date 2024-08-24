#include "../../include/Pipelines/PipelineCloud.h"
#include "../../include/Containers/VBO.h"
#include "../../include/Containers/EBO.h"
#include <chrono>
#include <glm/gtx/string_cast.hpp>
#include "../../include/Helpers/IBLBuilder.h"

Prisma::PipelineCloud::PipelineCloud()
{
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/CloudPipeline/vertex.glsl", "../../../Engine/Shaders/CloudPipeline/fragment.glsl");

	m_shader->use();

	m_cameraPos = m_shader->getUniformPosition("cameraPos");

	m_lightPos = m_shader->getUniformPosition("lightDir");

	m_timePos = m_shader->getUniformPosition("time");

	m_texturePos = m_shader->getUniformPosition("screenTexture");

	Prisma::SceneLoader loader;
	auto scene = loader.loadScene("../../../Resources/Cube/cube.gltf", { true });

	m_mesh = std::dynamic_pointer_cast<Prisma::Mesh>(scene->root->children()[0]);
	m_vao = std::make_shared<Prisma::VAO>();
	m_vao->bind();

	auto vbo = std::make_shared<Prisma::VBO>();

	auto ebo = std::make_shared<Prisma::EBO>();

	m_verticesData = m_mesh->verticesData();

	vbo->writeData(m_verticesData.vertices.size() * sizeof(Prisma::Mesh::Vertex), m_verticesData.vertices.data(), GL_DYNAMIC_DRAW);
	ebo->writeData(m_verticesData.indices.size() * sizeof(unsigned int), m_verticesData.indices.data(), GL_DYNAMIC_DRAW);

	m_vao->addAttribPointer(0, 3, sizeof(Prisma::Mesh::Vertex), (void*)0);

	m_vao->resetVao();

	// Initialize the start time
	m_start = std::chrono::system_clock::now();
}

void Prisma::PipelineCloud::render(std::shared_ptr<Prisma::FBO> texture, std::shared_ptr<Prisma::FBO> raw)
{
	glClear(GL_DEPTH_BUFFER_BIT);

	m_shader->use();

	m_shader->setVec3(m_cameraPos, currentGlobalScene->camera->position());
	if (currentGlobalScene->dirLights.size() > 0) {
		m_shader->setVec3(m_lightPos, glm::normalize(currentGlobalScene->dirLights[0]->type().direction));
	}

	// Calculate elapsed time since the first render call
	auto now = std::chrono::system_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(now - m_start).count();

	m_shader->setInt64(m_texturePos, texture->texture());

	m_shader->setFloat(m_timePos, static_cast<float>(elapsedTime));

	Prisma::IBLBuilder::getInstance().renderQuad();

}
