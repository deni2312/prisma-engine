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

Prisma::CloudComponent::CloudComponent()
{

}

void Prisma::CloudComponent::ui()
{
	std::vector<ComponentType> components;

	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "CoverageScale", &m_coverageScale));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "AmbientColorScale", &m_ambientColorScale));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "CloudType", &m_cloudType));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "LowFrequencyNoiseScale", &m_lowFrequencyNoiseScale));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "HighFrequencyNoiseScale", &m_highFrequencyNoiseScale));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "HighFrequencyNoiseErodeMuliplier", &m_highFrequencyNoiseErodeMuliplier));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "HighFrequencyHeightTransitionMultiplier", &m_highFrequencyHeightTransitionMultiplier));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "AnvilBias", &m_anvilBias));

	// Add vec3 components
	components.push_back(std::make_tuple(Prisma::Component::TYPES::VEC3, "CloudColor", &m_cloudColor));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::VEC3, "WindDirection", &m_windDirection));

	// Add more float components
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "RainCloudAbsorptionGain", &m_rainCloudAbsorptionGain));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "CloudAttenuationScale", &m_cloudAttenuationScale));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "PhaseEccentricity", &m_phaseEccentricity));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "PhaseSilverLiningIntensity", &m_phaseSilverLiningIntensity));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "PhaseSilverLiningSpread", &m_phaseSilverLiningSpread));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "ConeSpreadMultiplier", &m_coneSpreadMultplier));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "ShadowSampleConeSpreadMultiplier", &m_shadowSampleConeSpreadMultiplier));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "PowderedSugarEffectMultiplier", &m_powderedSugarEffectMultiplier));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "ToneMapperEyeExposure", &m_toneMapperEyeExposure));

	// Add bool components
	components.push_back(std::make_tuple(Prisma::Component::TYPES::BOOL, "IgnoreDetailNoise", &m_ignoreDetailNoise));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::BOOL, "UseEarlyExitAtFullOpacity", &m_useEarlyExitAtFullOpacity));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::BOOL, "UseBayerFilter", &m_useBayerFilter));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::BOOL, "RenderDirectlyToFullscreen", &m_renderDirectlyToFullscreen));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::BOOL, "RenderActualQuarterResolutionBuffer", &m_renderActualQuarterResolutionBuffer));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::BOOL, "MoveSunManually", &m_moveSunManually));

	// Add raymarch settings
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "MaxRenderDistance", &m_maxRenderDistance));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "MaxHorizontalSampleCount", &m_maxHorizontalSampleCount));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "MaxVerticalSampleCount", &m_maxVerticalSampleCount));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "EarthRadius", &m_earthRadius));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "VolumetricCloudsStartRadius", &m_volumetricCloudsStartRadius));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "VolumetricCloudsEndRadius", &m_volumetricCloudsEndRadius));

	// Add wind settings
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "WindUpwardBias", &m_windUpwardBias));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "CloudSpeed", &m_cloudSpeed));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "CloudTopOffset", &m_cloudTopOffset));

	// Add sun settings
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "SunIntensity", &m_sunIntensity));
	components.push_back(std::make_tuple(Prisma::Component::TYPES::FLOAT, "SunPitch", &m_sunPitch));

	ComponentType componentButton;
	auto startButton = [&]() {
		start();
	};
	componentButton = std::make_tuple(Prisma::Component::TYPES::BUTTON, "UI clouds", &startButton);
	for (const auto& component : components) {
		addGlobal(component);
	}
	addGlobal(componentButton);
}

void Prisma::CloudComponent::updateRender()
{
	if (cloudShader) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

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

	m_resolution = glm::vec2(settings.width, settings.height);

	m_fbo = std::make_shared<Prisma::FBO>(fboData);

	generateNoise();
	cloudShader->use();

	m_modelPos = cloudShader->getUniformPosition("model");

	m_lightPos = cloudShader->getUniformPosition("sunPosition");

	m_perlworlPos = cloudShader->getUniformPosition("perlworl");

	m_worlPos = cloudShader->getUniformPosition("worl");

	m_timePos = cloudShader->getUniformPosition("time");

	m_resolutionPos = cloudShader->getUniformPosition("resolution");

	m_noisePos = cloudShader->getUniformPosition("uNoise");

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
	cloudShader->setVec2(m_resolutionPos, m_resolution);

	cloudShader->setInt64(m_perlworlPos, m_perlinWorley);
	cloudShader->setInt64(m_worlPos, m_worley);

	// Setting the uniforms
	cloudShader->setMat4(m_invViewPos, glm::inverse(currentGlobalScene->camera->matrix()));                  // Inverse View matrix
	cloudShader->setMat4(m_invProjPos, glm::inverse(currentProjection));                  // Inverse Projection matrix
	cloudShader->setVec2(m_resolutionPos, m_resolution);            // Resolution

	cloudShader->setFloat(m_coverageScalePos, m_coverageScale);      // Cloud coverage scale
	cloudShader->setFloat(m_cloudTypePos, m_cloudType);              // Cloud type
	cloudShader->setFloat(m_lowFrequencyNoiseScalePos, m_lowFrequencyNoiseScale);  // Low frequency noise scale
	cloudShader->setBool(m_ignoreDetailNoisePos, m_ignoreDetailNoise);  // Ignore detail noise
	cloudShader->setFloat(m_highFrequencyNoiseScalePos, m_highFrequencyNoiseScale);  // High frequency noise scale
	cloudShader->setFloat(m_highFrequencyNoiseErodeMuliplierPos, m_highFrequencyNoiseErodeMuliplier);  // High frequency noise erosion multiplier
	cloudShader->setFloat(m_highFrequencyHeightTransitionMultiplierPos, m_highFrequencyHeightTransitionMultiplier);  // High frequency height transition multiplier
	cloudShader->setFloat(m_anvilBiasPos, m_anvilBias);              // Anvil bias

	cloudShader->setVec3(m_cloudColorPos, m_cloudColor);             // Cloud color
	cloudShader->setFloat(m_sunIntensityPos, m_sunIntensity);        // Sun intensity
	cloudShader->setFloat(m_ambientColorScalePos, m_ambientColorScale);  // Ambient color scale
	cloudShader->setFloat(m_rainCloudAbsorptionGainPos, m_rainCloudAbsorptionGain);  // Rain cloud absorption gain
	cloudShader->setFloat(m_cloudAttenuationScalePos, m_cloudAttenuationScale);  // Cloud attenuation scale
	cloudShader->setFloat(m_phaseEccentricityPos, m_phaseEccentricity);  // Phase eccentricity
	cloudShader->setFloat(m_phaseSilverLiningIntensityPos, m_phaseSilverLiningIntensity);  // Phase silver lining intensity
	cloudShader->setFloat(m_phaseSilverLiningSpreadPos, m_phaseSilverLiningSpread);  // Phase silver lining spread
	cloudShader->setFloat(m_coneSpreadMultplierPos, m_coneSpreadMultplier);  // Cone spread multiplier
	cloudShader->setFloat(m_shadowSampleConeSpreadMultiplierPos, m_shadowSampleConeSpreadMultiplier);  // Shadow sample cone spread multiplier
	cloudShader->setFloat(m_powderedSugarEffectMultiplierPos, m_powderedSugarEffectMultiplier);  // Powdered sugar effect multiplier
	cloudShader->setFloat(m_toneMapperEyeExposurePos, m_toneMapperEyeExposure);  // Tone mapper eye exposure

	cloudShader->setFloat(m_maxRenderDistancePos, m_maxRenderDistance);  // Maximum render distance
	cloudShader->setFloat(m_maxHorizontalSampleCountPos, m_maxHorizontalSampleCount);  // Max horizontal sample count
	cloudShader->setFloat(m_maxVerticalSampleCountPos, m_maxVerticalSampleCount);  // Max vertical sample count
	cloudShader->setBool(m_useEarlyExitAtFullOpacityPos, m_useEarlyExitAtFullOpacity);  // Early exit at full opacity
	cloudShader->setBool(m_useBayerFilterPos, m_useBayerFilter);  // Bayer filter
	cloudShader->setFloat(m_earthRadiusPos, m_earthRadius);       // Earth radius
	cloudShader->setFloat(m_volumetricCloudsStartRadiusPos, m_volumetricCloudsStartRadius);  // Volumetric clouds start radius
	cloudShader->setFloat(m_volumetricCloudsEndRadiusPos, m_volumetricCloudsEndRadius);  // Volumetric clouds end radius

	cloudShader->setVec3(m_windDirectionPos, m_windDirection);     // Wind direction
	cloudShader->setFloat(m_windUpwardBiasPos, m_windUpwardBias);  // Wind upward bias
	cloudShader->setFloat(m_cloudSpeedPos, m_cloudSpeed);          // Cloud speed
	cloudShader->setFloat(m_cloudTopOffsetPos, m_cloudTopOffset);  // Cloud top offset
}
