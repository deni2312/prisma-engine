#include "../../include/Pipelines/PipelineCloud.h"
#include "../../include/Containers/VBO.h"
#include "../../include/Containers/EBO.h"
#include <chrono>
#include <glm/gtx/string_cast.hpp>
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/IBLBuilder.h"

std::shared_ptr<Prisma::PipelineCloud> Prisma::PipelineCloud::instance = nullptr;

Prisma::PipelineCloud::PipelineCloud()
{
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/CloudPipeline/vertex.glsl", "../../../Engine/Shaders/CloudPipeline/fragment.glsl");

	m_shader->use();

	m_modelPos = m_shader->getUniformPosition("model");

	m_cameraPos = m_shader->getUniformPosition("cameraPos");

	m_lightPos = m_shader->getUniformPosition("lightDir");

	m_timePos = m_shader->getUniformPosition("iTime");

	m_bboxMinPos = m_shader->getUniformPosition("bboxMin");

	m_bboxMaxPos = m_shader->getUniformPosition("bboxMax");

	m_inverseModelPos = m_shader->getUniformPosition("inverseModelMatrix");

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

	auto settings = Prisma::SettingsLoader::instance().getSettings();

	Prisma::FBO::FBOData fboData;
	fboData.width = settings.width;
	fboData.height = settings.height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	fboData.name = "CLOUDS";

	m_fbo = std::make_shared<Prisma::FBO>(fboData);

	// Initialize the start time
	m_start = std::chrono::system_clock::now();
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
	m_fbo->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_vao->bind();

	m_shader->use();

	m_shader->setMat4(m_modelPos, m_mesh->finalMatrix());

	m_shader->setVec3(m_cameraPos, currentGlobalScene->camera->position());
	if (currentGlobalScene->dirLights.size() > 0) {
		m_shader->setVec3(m_lightPos, glm::normalize(currentGlobalScene->dirLights[0]->type().direction));
	}

	// Calculate elapsed time since the first render call
	auto now = std::chrono::system_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start).count();

	m_shader->setFloat(m_timePos, static_cast<float>(elapsedTime)/1000.0f);

	m_shader->setVec3(m_bboxMinPos, m_mesh->finalMatrix() * glm::vec4(m_mesh->aabbData().min, 1.0f));
	m_shader->setVec3(m_bboxMaxPos, m_mesh->finalMatrix() * glm::vec4(m_mesh->aabbData().max, 1.0f));

	m_shader->setMat4(m_inverseModelPos, glm::inverse(m_mesh->finalMatrix()));

	Prisma::IBLBuilder::getInstance().renderQuad();
	m_fbo->unbind();
}