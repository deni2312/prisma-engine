#include "../../include/Components/CloudComponent.h"
#include "../../include/Containers/VBO.h"
#include "../../include/Containers/EBO.h"
#include <chrono>
#include <glm/gtx/string_cast.hpp>
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/IBLBuilder.h"
#include "../../../GUI/include/TextureInfo.h"

static std::shared_ptr<Prisma::Shader> cloudShader = nullptr;
static std::shared_ptr<Prisma::Shader> noiseShader = nullptr;
static std::shared_ptr<Prisma::Shader> worleyShader = nullptr;

Prisma::CloudComponent::CloudComponent()
{

}

void Prisma::CloudComponent::updateRender()
{
	if (cloudShader) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		cloudShader->use();

		cloudShader->setVec3(m_cameraPos, currentGlobalScene->camera->position());
		if (currentGlobalScene->dirLights.size() > 0) {
			cloudShader->setVec3(m_lightPos, glm::normalize(currentGlobalScene->dirLights[0]->type().direction));
		}

		// Calculate elapsed time since the first render call
		auto now = std::chrono::system_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start).count();

		cloudShader->setFloat(m_timePos, static_cast<float>(elapsedTime) / 1000.0f);

		cloudShader->setVec2(m_resolutionPos, m_resolution);

		cloudShader->setInt64(m_noisePos,m_textureNoise->id());

		Prisma::IBLBuilder::getInstance().renderQuad();
	}
}

void Prisma::CloudComponent::start()
{
	Prisma::Component::start();
	if (cloudShader == nullptr) {
		cloudShader = std::make_shared<Shader>("../../../Engine/Shaders/CloudPipeline/vertex.glsl", "../../../Engine/Shaders/CloudPipeline/fragment.glsl");
		noiseShader = std::make_shared<Shader>("../../../Engine/Shaders/NoisePipeline/vertex.glsl", "../../../Engine/Shaders/NoisePipeline/fragment.glsl");
		worleyShader = std::make_shared<Shader>("../../../Engine/Shaders/NoisePipeline/compute.glsl");
	}
	//generateNoise();
	cloudShader->use();

	m_modelPos = cloudShader->getUniformPosition("model");

	m_cameraPos = cloudShader->getUniformPosition("cameraPos");

	m_lightPos = cloudShader->getUniformPosition("lightDir");

	m_timePos = cloudShader->getUniformPosition("iTime");

	m_resolutionPos = cloudShader->getUniformPosition("resolution");

	m_noisePos = cloudShader->getUniformPosition("uNoise");

	auto settings = Prisma::SettingsLoader::getInstance().getSettings();

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

	m_textureNoise = std::make_shared<Prisma::Texture>();
	m_textureNoise->loadTexture("../../../Resources/res/noise.png");

	m_fbo->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	noiseShader->use();
	Prisma::IBLBuilder::getInstance().renderQuad();
	m_fbo->unbind();

}

void Prisma::CloudComponent::generateNoise()
{
	worleyShader->use();
	glm::ivec3 size(1024, 1024, 1024);
	// Generate a texture ID
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Bind the texture to a texture unit
	glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Allocate storage for the texture (initially empty)
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	m_worley = glGetTextureHandleARB(textureID);
	glMakeTextureHandleResidentARB(m_worley);

	worleyShader->setVec2(worleyShader->getUniformPosition("texSize"), size);
	worleyShader->setInt64(worleyShader->getUniformPosition("worleyTexture"), m_worley);

	worleyShader->dispatchCompute(size);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Prisma::TextureInfo::getInstance().add({ textureID ,"WORLEY" });
	glBindTexture(GL_TEXTURE_2D, 0);

}
