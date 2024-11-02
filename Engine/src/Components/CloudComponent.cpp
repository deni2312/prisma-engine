#include "../../include/Components/CloudComponent.h"
#include "../../include/Containers/VBO.h"
#include "../../include/Containers/EBO.h"
#include <chrono>
#include <glm/gtx/string_cast.hpp>
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/PrismaRender.h"
#include "../../../GUI/include/TextureInfo.h"

Prisma::CloudComponent::CloudComponent() : Component{}
{
	name("Cloud");
}

void Prisma::CloudComponent::ui()
{
	std::vector<ComponentType> components;

	components.push_back(std::make_tuple(TYPES::FLOAT, "CoverageScale", &m_cloudSSBO.coverageScale));
	components.push_back(std::make_tuple(TYPES::FLOAT, "AmbientColorScale", &m_cloudSSBO.ambientColorScale));
	components.push_back(std::make_tuple(TYPES::FLOAT, "CloudType", &m_cloudSSBO.cloudType));
	components.push_back(std::make_tuple(TYPES::FLOAT, "LowFrequencyNoiseScale",
	                                     &m_cloudSSBO.lowFrequencyNoiseScale));
	components.push_back(std::make_tuple(TYPES::FLOAT, "HighFrequencyNoiseScale",
	                                     &m_cloudSSBO.highFrequencyNoiseScale));
	components.push_back(std::make_tuple(TYPES::FLOAT, "HighFrequencyNoiseErodeMuliplier",
	                                     &m_cloudSSBO.highFrequencyNoiseErodeMuliplier));
	components.push_back(std::make_tuple(TYPES::FLOAT, "HighFrequencyHeightTransitionMultiplier",
	                                     &m_cloudSSBO.highFrequencyHeightTransitionMultiplier));
	components.push_back(std::make_tuple(TYPES::FLOAT, "AnvilBias", &m_cloudSSBO.anvilBias));

	// Add vec3 components
	components.push_back(std::make_tuple(TYPES::VEC3, "CloudColor", &m_cloudSSBO.cloudColor));
	components.push_back(std::make_tuple(TYPES::VEC3, "WindDirection", &m_cloudSSBO.windDirection));

	// Add more float components
	components.push_back(std::make_tuple(TYPES::FLOAT, "RainCloudAbsorptionGain",
	                                     &m_cloudSSBO.rainCloudAbsorptionGain));
	components.push_back(std::make_tuple(TYPES::FLOAT, "CloudAttenuationScale", &m_cloudSSBO.cloudAttenuationScale));
	components.push_back(std::make_tuple(TYPES::FLOAT, "PhaseEccentricity", &m_cloudSSBO.phaseEccentricity));
	components.push_back(std::make_tuple(TYPES::FLOAT, "PhaseSilverLiningIntensity",
	                                     &m_cloudSSBO.phaseSilverLiningIntensity));
	components.push_back(std::make_tuple(TYPES::FLOAT, "PhaseSilverLiningSpread",
	                                     &m_cloudSSBO.phaseSilverLiningSpread));
	components.push_back(std::make_tuple(TYPES::FLOAT, "ConeSpreadMultiplier", &m_cloudSSBO.coneSpreadMultplier));
	components.push_back(std::make_tuple(TYPES::FLOAT, "ShadowSampleConeSpreadMultiplier",
	                                     &m_cloudSSBO.shadowSampleConeSpreadMultiplier));
	components.push_back(std::make_tuple(TYPES::FLOAT, "PowderedSugarEffectMultiplier",
	                                     &m_cloudSSBO.powderedSugarEffectMultiplier));
	components.push_back(std::make_tuple(TYPES::FLOAT, "ToneMapperEyeExposure", &m_cloudSSBO.toneMapperEyeExposure));

	// Add bool components
	components.push_back(std::make_tuple(TYPES::BOOL, "IgnoreDetailNoise", &m_cloudSSBO.ignoreDetailNoise));
	components.push_back(std::make_tuple(TYPES::BOOL, "UseEarlyExitAtFullOpacity",
	                                     &m_cloudSSBO.useEarlyExitAtFullOpacity));
	components.push_back(std::make_tuple(TYPES::BOOL, "UseBayerFilter", &m_cloudSSBO.useBayerFilter));

	// Add raymarch settings
	components.push_back(std::make_tuple(TYPES::FLOAT, "MaxRenderDistance", &m_cloudSSBO.maxRenderDistance));
	components.push_back(std::make_tuple(TYPES::FLOAT, "MaxHorizontalSampleCount",
	                                     &m_cloudSSBO.maxHorizontalSampleCount));
	components.push_back(std::make_tuple(TYPES::FLOAT, "MaxVerticalSampleCount",
	                                     &m_cloudSSBO.maxVerticalSampleCount));
	components.push_back(std::make_tuple(TYPES::FLOAT, "EarthRadius", &m_cloudSSBO.earthRadius));
	components.push_back(std::make_tuple(TYPES::FLOAT, "VolumetricCloudsStartRadius",
	                                     &m_cloudSSBO.volumetricCloudsStartRadius));
	components.push_back(std::make_tuple(TYPES::FLOAT, "VolumetricCloudsEndRadius",
	                                     &m_cloudSSBO.volumetricCloudsEndRadius));

	// Add wind settings
	components.push_back(std::make_tuple(TYPES::FLOAT, "WindUpwardBias", &m_cloudSSBO.windUpwardBias));
	components.push_back(std::make_tuple(TYPES::FLOAT, "CloudSpeed", &m_cloudSSBO.cloudSpeed));
	components.push_back(std::make_tuple(TYPES::FLOAT, "CloudTopOffset", &m_cloudSSBO.cloudTopOffset));

	// Add sun settings
	components.push_back(std::make_tuple(TYPES::FLOAT, "SunIntensity", &m_cloudSSBO.sunIntensity));

	ComponentType componentButton;
	m_startButton = [&]()
	{
		if (!isStart())
		{
			start();
		}
	};
	componentButton = std::make_tuple(TYPES::BUTTON, "UI clouds", &m_startButton);

	m_updateButton = [&]()
	{
		if (isStart())
		{
			m_ssbo->modifyData(0, sizeof(CloudSSBO), &m_cloudSSBO);
		}
	};
	ComponentType updateButton = std::make_tuple(TYPES::BUTTON, "Update Clouds", &m_updateButton);
	for (const auto& component : components)
	{
		addGlobal(component);
	}
	addGlobal(componentButton);

	addGlobal(updateButton);
}

void Prisma::CloudComponent::updatePostRender(std::shared_ptr<FBO> fbo)
{
	if (m_cloudShader)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(0, 0, m_settings.width / m_downscale, m_settings.height / m_downscale);

		m_fbo->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_cloudShader->use();

		setVariables();

		PrismaRender::getInstance().renderQuad();
		m_fbo->unbind();
		fbo->bind();
		glDepthMask(GL_FALSE);
		glViewport(0, 0, m_settings.width, m_settings.height);
		m_upscaleShader->use();
		m_upscaleShader->setInt64(m_upscalePos, m_fbo->texture());
		m_upscaleShader->setVec2(m_resUpscalePos, glm::vec2(m_settings.width, m_settings.height));
		m_upscaleShader->setInt(m_factorPos, m_downscale);
		PrismaRender::getInstance().renderQuad();
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
}

void Prisma::CloudComponent::start()
{
	Component::start();
	m_settings = SettingsLoader::getInstance().getSettings();
	m_cloudShader = std::make_shared<Shader>("../../../Engine/Shaders/CloudPipeline/vertex.glsl",
	                                         "../../../Engine/Shaders/CloudPipeline/fragment.glsl");
	m_noiseShader = std::make_shared<Shader>("../../../Engine/Shaders/NoisePipeline/vertex.glsl",
	                                         "../../../Engine/Shaders/NoisePipeline/fragment.glsl");
	m_worleyShader = std::make_shared<Shader>("../../../Engine/Shaders/NoisePipeline/computeWorley.glsl");
	m_perlinWorleyShader = std::make_shared<Shader>("../../../Engine/Shaders/NoisePipeline/computePerlinWorley.glsl");
	m_weatherShader = std::make_shared<Shader>("../../../Engine/Shaders/NoisePipeline/computeWeather.glsl");
	m_upscaleShader = std::make_shared<Shader>("../../../Engine/Shaders/UpscalePipeline/vertex.glsl",
	                                           "../../../Engine/Shaders/UpscalePipeline/fragment.glsl");

	FBO::FBOData fboData;
	fboData.width = m_settings.width / m_downscale;
	fboData.height = m_settings.height / m_downscale;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	fboData.name = "CLOUDS";

	m_cloudSSBO.resolution = glm::vec2(m_settings.width / m_downscale, m_settings.height / m_downscale);

	m_fbo = std::make_shared<FBO>(fboData);

	generateNoise();

	m_upscaleShader->use();
	m_upscalePos = m_upscaleShader->getUniformPosition("screenTexture");
	m_resUpscalePos = m_upscaleShader->getUniformPosition("resolution");
	m_factorPos = m_upscaleShader->getUniformPosition("factor");

	m_cloudShader->use();

	m_modelPos = m_cloudShader->getUniformPosition("model");

	m_lightPos = m_cloudShader->getUniformPosition("sunPosition");

	m_perlworlPos = m_cloudShader->getUniformPosition("perlworl");

	m_worlPos = m_cloudShader->getUniformPosition("worl");

	m_timePos = m_cloudShader->getUniformPosition("time");

	m_invViewPos = m_cloudShader->getUniformPosition("invView");

	m_invProjPos = m_cloudShader->getUniformPosition("invProj");

	m_camPos = m_cloudShader->getUniformPosition("camPos");
	// Initialize the start time
	m_start = std::chrono::system_clock::now();
	m_fbo->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_noiseShader->use();
	PrismaRender::getInstance().renderQuad();
	m_fbo->unbind();

	m_ssbo = std::make_shared<SSBO>(11);
	m_ssbo->resize(sizeof(CloudSSBO));

	m_ssbo->modifyData(0, sizeof(CloudSSBO), &m_cloudSSBO);
}

void Prisma::CloudComponent::generateNoise()
{
	generateWorley();
	generatePerlinWorley();
	generateWeather();
}

void Prisma::CloudComponent::generateWorley()
{
	//GENERATE WORLEY
	m_worleyShader->use();
	glm::ivec3 size(32, 32, 32);
	// Generate a texture ID
	unsigned int textureId;

	glGenTextures(1, &textureId);

	// 2. Bind the texture as a 3D texture
	glBindTexture(GL_TEXTURE_3D, textureId);

	// 3. Allocate storage for the 3D texture with RGBA8 format
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, size.x, size.y, size.z, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// 4. Set texture parameters (filtering and wrapping modes)
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindImageTexture(0, textureId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

	m_worley = glGetTextureHandleARB(textureId);
	glMakeTextureHandleResidentARB(m_worley);

	m_worleyShader->setInt64(m_worleyShader->getUniformPosition("outVolTex"), m_worley);

	m_worleyShader->dispatchCompute(size);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Prisma::CloudComponent::generatePerlinWorley()
{
	//GENERATE PERLIN WORLEY
	m_perlinWorleyShader->use();
	glm::ivec3 size(128, 128, 128);
	// Generate a texture ID
	unsigned int textureId;

	glGenTextures(1, &textureId);

	// 2. Bind the texture as a 3D texture
	glBindTexture(GL_TEXTURE_3D, textureId);

	// 3. Allocate storage for the 3D texture with RGBA8 format
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, size.x, size.y, size.z, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	// 4. Set texture parameters (filtering and wrapping modes)
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindImageTexture(0, textureId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

	m_perlinWorley = glGetTextureHandleARB(textureId);
	glMakeTextureHandleResidentARB(m_perlinWorley);

	m_perlinWorleyShader->setInt64(m_perlinWorleyShader->getUniformPosition("outVolTex"), m_perlinWorley);

	m_perlinWorleyShader->dispatchCompute(size);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Prisma::CloudComponent::generateWeather()
{
	//GENERATE WEATHER
	m_weatherShader->use();
	glm::ivec3 size(256, 256, 1);
	// Generate a texture ID
	unsigned int textureId;

	glGenTextures(1, &textureId);

	// 2. Bind the texture as a 3D texture
	glBindTexture(GL_TEXTURE_2D, textureId);

	// 3. Allocate storage for the 3D texture with RGBA8 format
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindImageTexture(0, textureId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

	m_weather = glGetTextureHandleARB(textureId);
	glMakeTextureHandleResidentARB(m_weather);

	m_weatherShader->setInt64(m_weatherShader->getUniformPosition("outWeatherTex"), m_weather);

	m_weatherShader->dispatchCompute(size);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Prisma::CloudComponent::setVariables()
{
	m_cloudShader->setVec3(m_camPos, currentGlobalScene->camera->position());
	if (currentGlobalScene->dirLights.size() > 0)
	{
		m_cloudShader->setVec3(m_lightPos, normalize(
			                       currentGlobalScene->dirLights[0]->parent()->finalMatrix() * currentGlobalScene->
			                       dirLights[0]->type().direction));
	}
	// Calculate elapsed time since the first render call
	auto now = std::chrono::system_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start).count();

	m_cloudShader->setFloat(m_timePos, static_cast<int>(static_cast<float>(elapsedTime)) / 1000.0f);

	m_cloudShader->setInt64(m_perlworlPos, m_perlinWorley);
	m_cloudShader->setInt64(m_worlPos, m_worley);

	// Setting the uniforms
	m_cloudShader->setMat4(m_invViewPos, glm::inverse(currentGlobalScene->camera->matrix())); // Inverse View matrix
	m_cloudShader->setMat4(m_invProjPos, glm::inverse(currentProjection)); // Inverse Projection matrix
}
