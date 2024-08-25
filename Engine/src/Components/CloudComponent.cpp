#include "../../include/Components/CloudComponent.h"
#include "../../include/Containers/VBO.h"
#include "../../include/Containers/EBO.h"
#include <chrono>
#include <glm/gtx/string_cast.hpp>
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/IBLBuilder.h"

Prisma::CloudComponent::CloudComponent()
{

}

void Prisma::CloudComponent::updateRender()
{

	m_shader->use();

	m_shader->setVec3(m_cameraPos, currentGlobalScene->camera->position());
	if (currentGlobalScene->dirLights.size() > 0) {
		m_shader->setVec3(m_lightPos, glm::normalize(currentGlobalScene->dirLights[0]->type().direction));
	}

	// Calculate elapsed time since the first render call
	auto now = std::chrono::system_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start).count();

	m_shader->setFloat(m_timePos, static_cast<float>(elapsedTime) / 1000.0f);

	m_shader->setVec2(m_resolutionPos, m_resolution);

	Prisma::IBLBuilder::getInstance().renderQuad();
}

void Prisma::CloudComponent::start()
{
	Prisma::Component::start();
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/CloudPipeline/vertex.glsl", "../../../Engine/Shaders/CloudPipeline/fragment.glsl");

	m_shader->use();

	m_modelPos = m_shader->getUniformPosition("model");

	m_cameraPos = m_shader->getUniformPosition("cameraPos");

	m_lightPos = m_shader->getUniformPosition("lightDir");

	m_timePos = m_shader->getUniformPosition("iTime");

	m_resolutionPos = m_shader->getUniformPosition("resolution");

	auto settings = Prisma::SettingsLoader::instance().getSettings();

	Prisma::FBO::FBOData fboData;
	fboData.width = settings.width;
	fboData.height = settings.height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	fboData.name = "CLOUDS";

	m_resolution = glm::vec2(settings.width, settings.height);

	m_fbo = std::make_shared<Prisma::FBO>(fboData);

	// Initialize the start time
	m_start = std::chrono::system_clock::now();
}
