#include "../../include/Components/CloudComponent.h"
#include "../../include/Containers/VBO.h"
#include "../../include/Containers/EBO.h"
#include <chrono>
#include <glm/gtx/string_cast.hpp>
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/IBLBuilder.h"

static std::shared_ptr<Prisma::Shader> cloudShader = nullptr;
static std::shared_ptr<Prisma::Shader> noiseShader = nullptr;
static std::shared_ptr<Prisma::Shader> worleyShader = nullptr;

Prisma::CloudComponent::CloudComponent()
{

}

void Prisma::CloudComponent::updateRender()
{
	if (cloudShader) {
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
	generateNoise();
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
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, size.x, size.y, size.z, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindImageTexture(0, tex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

	auto id = glGetTextureHandleARB(tex);
	glMakeTextureHandleResidentARB(id);
	int groupSizeX = (size.x + 7) / 8;  // Add 7 to ensure rounding up
	int groupSizeY = (size.y + 7) / 8;
	int groupSizeZ = (size.z + 7) / 8;
	worleyShader->setVec3(worleyShader->getUniformPosition("texSize"), size);
	worleyShader->dispatchCompute({ groupSizeX ,groupSizeY ,groupSizeZ });
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
