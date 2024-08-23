#include "../../include/Pipelines/PipelineCloud.h"
#include "../../include/Containers/VBO.h"
#include "../../include/Containers/EBO.h"

std::shared_ptr<Prisma::PipelineCloud> Prisma::PipelineCloud::instance = nullptr;

Prisma::PipelineCloud::PipelineCloud()
{
    m_shader = std::make_shared<Shader>("../../../Engine/Shaders/CloudPipeline/vertex.glsl", "../../../Engine/Shaders/CloudPipeline/fragment.glsl");

	m_shader->use();

	m_modelPos = m_shader->getUniformPosition("model");

    Prisma::SceneLoader loader;
    auto scene = loader.loadScene("../../../Resources/Cube/cube.gltf", {true});
    m_root = scene->root;
	m_vao = std::make_shared<Prisma::VAO>();
	m_vao->bind();

	auto vbo = std::make_shared<Prisma::VBO>();

	auto ebo = std::make_shared<Prisma::EBO>();

	m_verticesData = std::dynamic_pointer_cast<Prisma::Mesh>(m_root->children()[0])->verticesData();

	vbo->writeData(m_verticesData.vertices.size()*sizeof(Prisma::Mesh::Vertex), m_verticesData.vertices.data(), GL_DYNAMIC_DRAW);
	ebo->writeData(m_verticesData.indices.size() * sizeof(unsigned int), m_verticesData.indices.data(), GL_DYNAMIC_DRAW);

	m_vao->addAttribPointer(0, 3, sizeof(Prisma::Mesh::Vertex), (void*)0);

	m_vao->resetVao();
}

Prisma::PipelineCloud& Prisma::PipelineCloud::getInstance()
{
	if (!instance) {
		instance = std::make_shared<PipelineCloud>();
	}
	return *instance;
}

void Prisma::PipelineCloud::render()
{
	m_vao->bind();

	m_shader->use();

	m_shader->setMat4(m_modelPos, m_root->children()[0]->finalMatrix());

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_verticesData.indices.size()), GL_UNSIGNED_INT, 0);
}
