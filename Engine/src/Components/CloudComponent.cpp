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
static std::shared_ptr<Prisma::Shader> perlinWorleyShader = nullptr;
static std::shared_ptr<Prisma::Shader> weatherShader = nullptr;

Prisma::CloudComponent::CloudComponent() : Prisma::Component{}
{
	name("Cloud");
}

void Prisma::CloudComponent::ui()
{
	std::vector<ComponentType> components;

	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "CoverageScale", &m_cloudSSBO.m_coverageScale));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "AmbientColorScale", &m_cloudSSBO.m_ambientColorScale));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "CloudType", &m_cloudSSBO.m_cloudType));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "LowFrequencyNoiseScale", &m_cloudSSBO.m_lowFrequencyNoiseScale));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "HighFrequencyNoiseScale", &m_cloudSSBO.m_highFrequencyNoiseScale));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "HighFrequencyNoiseErodeMuliplier", &m_cloudSSBO.m_highFrequencyNoiseErodeMuliplier));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "HighFrequencyHeightTransitionMultiplier", &m_cloudSSBO.m_highFrequencyHeightTransitionMultiplier));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "AnvilBias", &m_cloudSSBO.m_anvilBias));

	// Add vec3 components
	components.push_back(std::make_tuple(Prisma::Component::TYPES::VEC3, "CloudColor", &m_cloudSSBO.m_cloudColor));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::VEC3, "WindDirection", &m_cloudSSBO.m_windDirection));

	// Add more float components
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "RainCloudAbsorptionGain", &m_cloudSSBO.m_rainCloudAbsorptionGain));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "CloudAttenuationScale", &m_cloudSSBO.m_cloudAttenuationScale));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "PhaseEccentricity", &m_cloudSSBO.m_phaseEccentricity));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "PhaseSilverLiningIntensity", &m_cloudSSBO.m_phaseSilverLiningIntensity));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "PhaseSilverLiningSpread", &m_cloudSSBO.m_phaseSilverLiningSpread));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "ConeSpreadMultiplier", &m_cloudSSBO.m_coneSpreadMultplier));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "ShadowSampleConeSpreadMultiplier", &m_cloudSSBO.m_shadowSampleConeSpreadMultiplier));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "PowderedSugarEffectMultiplier", &m_cloudSSBO.m_powderedSugarEffectMultiplier));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "ToneMapperEyeExposure", &m_cloudSSBO.m_toneMapperEyeExposure));

	// Add bool components
	components.push_back(std::make_tuple(Prisma::Component::TYPES::BOOL, "IgnoreDetailNoise", &m_cloudSSBO.m_ignoreDetailNoise));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::BOOL, "UseEarlyExitAtFullOpacity", &m_cloudSSBO.m_useEarlyExitAtFullOpacity));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::BOOL, "UseBayerFilter", &m_cloudSSBO.m_useBayerFilter));

	// Add raymarch settings
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "MaxRenderDistance", &m_cloudSSBO.m_maxRenderDistance));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "MaxHorizontalSampleCount", &m_cloudSSBO.m_maxHorizontalSampleCount));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "MaxVerticalSampleCount", &m_cloudSSBO.m_maxVerticalSampleCount));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "EarthRadius", &m_cloudSSBO.m_earthRadius));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "VolumetricCloudsStartRadius", &m_cloudSSBO.m_volumetricCloudsStartRadius));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "VolumetricCloudsEndRadius", &m_cloudSSBO.m_volumetricCloudsEndRadius));

	// Add wind settings
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "WindUpwardBias", &m_cloudSSBO.m_windUpwardBias));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "CloudSpeed", &m_cloudSSBO.m_cloudSpeed));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "CloudTopOffset", &m_cloudSSBO.m_cloudTopOffset));

	// Add sun settings
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "SunIntensity", &m_cloudSSBO.m_sunIntensity));

	ComponentType componentButton;
	m_startButton = [&]() {
		if (!isStart()) {
			start();
		}
	};
	componentButton = std::make_tuple(Prisma::Component::TYPES::BUTTON, "UI clouds", &m_startButton);
	for (const auto& component : components) {
		addGlobal(component);
	}
	addGlobal(componentButton);
}

void Prisma::CloudComponent::updateRender()
{
	if (cloudShader) {

		cloudShader->use();

		setVariables();

		Prisma::IBLBuilder::getInstance().renderQuad();
	}
}

void Prisma::CloudComponent::start()
{
	Prisma::Component::start();
	if (cloudShader == nullptr) {
		cloudShader = std::make_shared<Shader>("../../../Engine/Shaders/CloudPipeline/vertex.glsl", "../../../Engine/Shaders/CloudPipeline/fragment.glsl");
		noiseShader = std::make_shared<Shader>("../../../Engine/Shaders/NoisePipeline/vertex.glsl", "../../../Engine/Shaders/NoisePipeline/fragment.glsl");
		worleyShader = std::make_shared<Shader>("../../../Engine/Shaders/NoisePipeline/computeWorley.glsl");
		perlinWorleyShader = std::make_shared<Shader>("../../../Engine/Shaders/NoisePipeline/computePerlinWorley.glsl");
		weatherShader = std::make_shared<Shader>("../../../Engine/Shaders/NoisePipeline/computeWeather.glsl");
	}
	auto settings = Prisma::SettingsLoader::getInstance().getSettings();

	Prisma::FBO::FBOData fboData;
	fboData.width = settings.width;
	fboData.height = settings.height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RGBA16F;
	fboData.internalType = GL_FLOAT;
	fboData.name = "CLOUDS";

	m_cloudSSBO.m_resolution = glm::vec2(settings.width, settings.height);

	m_fbo = std::make_shared<Prisma::FBO>(fboData);

	generateNoise();
	cloudShader->use();

	m_modelPos = cloudShader->getUniformPosition("model");

	m_lightPos = cloudShader->getUniformPosition("sunPosition");

	m_perlworlPos = cloudShader->getUniformPosition("perlworl");

	m_worlPos = cloudShader->getUniformPosition("worl");

	m_timePos = cloudShader->getUniformPosition("time");

	m_resolutionPos = cloudShader->getUniformPosition("resolution");

	m_modelPos = cloudShader->getUniformPosition("model");

	m_invViewPos = cloudShader->getUniformPosition("invView");
	m_invProjPos = cloudShader->getUniformPosition("invProj");
	m_timePos = cloudShader->getUniformPosition("time");  // Already exists
	m_resolutionPos = cloudShader->getUniformPosition("resolution");  // Already exists
	m_camPosPos = cloudShader->getUniformPosition("camPos");

	m_coverageScalePos = cloudShader->getUniformPosition("coverageScale");
	m_cloudTypePos = cloudShader->getUniformPosition("cloudType");
	m_lowFrequencyNoiseScalePos = cloudShader->getUniformPosition("lowFrequencyNoiseScale");
	m_ignoreDetailNoisePos = cloudShader->getUniformPosition("ignoreDetailNoise");
	m_highFrequencyNoiseScalePos = cloudShader->getUniformPosition("highFrequencyNoiseScale");
	m_highFrequencyNoiseErodeMuliplierPos = cloudShader->getUniformPosition("highFrequencyNoiseErodeMuliplier");
	m_highFrequencyHeightTransitionMultiplierPos = cloudShader->getUniformPosition("highFrequencyHeightTransitionMultiplier");
	m_anvilBiasPos = cloudShader->getUniformPosition("anvilBias");

	m_cloudColorPos = cloudShader->getUniformPosition("cloudColor");
	m_sunIntensityPos = cloudShader->getUniformPosition("sunIntensity");
	m_ambientColorScalePos = cloudShader->getUniformPosition("ambientColorScale");
	m_rainCloudAbsorptionGainPos = cloudShader->getUniformPosition("rainCloudAbsorptionGain");
	m_cloudAttenuationScalePos = cloudShader->getUniformPosition("cloudAttenuationScale");
	m_phaseEccentricityPos = cloudShader->getUniformPosition("phaseEccentricity");
	m_phaseSilverLiningIntensityPos = cloudShader->getUniformPosition("phaseSilverLiningIntensity");
	m_phaseSilverLiningSpreadPos = cloudShader->getUniformPosition("phaseSilverLiningSpread");
	m_coneSpreadMultplierPos = cloudShader->getUniformPosition("coneSpreadMultplier");
	m_shadowSampleConeSpreadMultiplierPos = cloudShader->getUniformPosition("shadowSampleConeSpreadMultiplier");
	m_powderedSugarEffectMultiplierPos = cloudShader->getUniformPosition("powderedSugarEffectMultiplier");
	m_toneMapperEyeExposurePos = cloudShader->getUniformPosition("toneMapperEyeExposure");

	m_maxRenderDistancePos = cloudShader->getUniformPosition("maxRenderDistance");
	m_maxHorizontalSampleCountPos = cloudShader->getUniformPosition("maxHorizontalSampleCount");
	m_maxVerticalSampleCountPos = cloudShader->getUniformPosition("maxVerticalSampleCount");
	m_useEarlyExitAtFullOpacityPos = cloudShader->getUniformPosition("useEarlyExitAtFullOpacity");
	m_useBayerFilterPos = cloudShader->getUniformPosition("useBayerFilter");
	m_earthRadiusPos = cloudShader->getUniformPosition("earthRadius");
	m_volumetricCloudsStartRadiusPos = cloudShader->getUniformPosition("volumetricCloudsStartRadius");
	m_volumetricCloudsEndRadiusPos = cloudShader->getUniformPosition("volumetricCloudsEndRadius");

	m_windDirectionPos = cloudShader->getUniformPosition("windDirection");
	m_windUpwardBiasPos = cloudShader->getUniformPosition("windUpwardBias");
	m_cloudSpeedPos = cloudShader->getUniformPosition("cloudSpeed");
	m_cloudTopOffsetPos = cloudShader->getUniformPosition("cloudTopOffset");
	// Initialize the start time
	m_start = std::chrono::system_clock::now();

	m_textureNoise = std::make_shared<Prisma::Texture>();
	m_textureNoise->loadTexture("../../../Resources/res/noise.png");

	m_fbo->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	noiseShader->use();
	Prisma::IBLBuilder::getInstance().renderQuad();
	m_fbo->unbind();

	m_ssbo = std::make_shared<Prisma::SSBO>(11);
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
	worleyShader->use();
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

	worleyShader->setInt64(worleyShader->getUniformPosition("outVolTex"), m_worley);

	worleyShader->dispatchCompute(size);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Prisma::CloudComponent::generatePerlinWorley()
{
	//GENERATE PERLIN WORLEY
	perlinWorleyShader->use();
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

	perlinWorleyShader->setInt64(perlinWorleyShader->getUniformPosition("outVolTex"), m_perlinWorley);

	perlinWorleyShader->dispatchCompute(size);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Prisma::CloudComponent::generateWeather()
{
	//GENERATE WEATHER
	weatherShader->use();
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

	weatherShader->setInt64(weatherShader->getUniformPosition("outWeatherTex"), m_weather);

	weatherShader->dispatchCompute(size);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Prisma::CloudComponent::setVariables()
{
	cloudShader->setVec3(m_camPosPos, currentGlobalScene->camera->position());
	if (currentGlobalScene->dirLights.size() > 0) {
		cloudShader->setVec3(m_lightPos, glm::normalize(currentGlobalScene->dirLights[0]->type().direction));
	}
	// Calculate elapsed time since the first render call
	auto now = std::chrono::system_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start).count();

	cloudShader->setFloat(m_timePos, (int)static_cast<float>(elapsedTime) / 1000.0f);

	cloudShader->setInt64(m_perlworlPos, m_perlinWorley);
	cloudShader->setInt64(m_worlPos, m_worley);

	// Setting the uniforms
	cloudShader->setMat4(m_invViewPos, glm::inverse(currentGlobalScene->camera->matrix()));                  // Inverse View matrix
	cloudShader->setMat4(m_invProjPos, glm::inverse(currentProjection));                  // Inverse Projection matrix
}
