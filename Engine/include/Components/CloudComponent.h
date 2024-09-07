#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Containers/FBO.h"
#include "../Pipelines/PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include <memory>
#include "../Containers/VAO.h"
#include <chrono>

namespace Prisma {
	class CloudComponent : public Component{
	public:
		CloudComponent();

		void ui() override;

		void updateRender() override;

		void start() override;

	private:
		unsigned int m_modelPos;

		unsigned int m_lightPos;

		unsigned int m_timePos;

		unsigned int m_resolutionPos;

		unsigned int m_noisePos;

		unsigned int m_invViewPos;
		unsigned int m_invProjPos;
		unsigned int m_camPosPos;

		unsigned int m_coverageScalePos;
		unsigned int m_cloudTypePos;
		unsigned int m_lowFrequencyNoiseScalePos;
		unsigned int m_ignoreDetailNoisePos;
		unsigned int m_highFrequencyNoiseScalePos;
		unsigned int m_highFrequencyNoiseErodeMuliplierPos;
		unsigned int m_highFrequencyHeightTransitionMultiplierPos;
		unsigned int m_anvilBiasPos;

		unsigned int m_cloudColorPos;
		unsigned int m_sunIntensityPos;
		unsigned int m_ambientColorScalePos;
		unsigned int m_rainCloudAbsorptionGainPos;
		unsigned int m_cloudAttenuationScalePos;
		unsigned int m_phaseEccentricityPos;
		unsigned int m_phaseSilverLiningIntensityPos;
		unsigned int m_phaseSilverLiningSpreadPos;
		unsigned int m_coneSpreadMultplierPos;
		unsigned int m_shadowSampleConeSpreadMultiplierPos;
		unsigned int m_powderedSugarEffectMultiplierPos;
		unsigned int m_toneMapperEyeExposurePos;

		unsigned int m_maxRenderDistancePos;
		unsigned int m_maxHorizontalSampleCountPos;
		unsigned int m_maxVerticalSampleCountPos;
		unsigned int m_useEarlyExitAtFullOpacityPos;
		unsigned int m_useBayerFilterPos;
		unsigned int m_earthRadiusPos;
		unsigned int m_volumetricCloudsStartRadiusPos;
		unsigned int m_volumetricCloudsEndRadiusPos;

		unsigned int m_windDirectionPos;
		unsigned int m_windUpwardBiasPos;
		unsigned int m_cloudSpeedPos;
		unsigned int m_cloudTopOffsetPos;

		unsigned int m_perlworlPos;
		unsigned int m_worlPos;

		GLuint m_downscalesq = 4 * 4;

		// Cloud model
		float m_coverageScale = 0.45f;
		float m_ambientColorScale = 0.7f;
		float m_cloudType = 0.8f;
		float m_lowFrequencyNoiseScale = 0.3f;
		bool m_ignoreDetailNoise = false;
		float m_highFrequencyNoiseScale = 0.3f;
		float m_highFrequencyNoiseErodeMuliplier = 0.19f;
		float m_highFrequencyHeightTransitionMultiplier = 10.0f;
		float m_anvilBias = 0.1f;

		// Cloud lighting
		glm::vec3 m_cloudColor = glm::vec3(1.0f, 1.0f, 1.0f);
		float m_rainCloudAbsorptionGain = 2.3f;
		float m_cloudAttenuationScale = 2.5f;
		float m_phaseEccentricity = 0.5f;
		float m_phaseSilverLiningIntensity = 0.15f;
		float m_phaseSilverLiningSpread = 0.5f;
		float m_coneSpreadMultplier = 0.2f;
		float m_shadowSampleConeSpreadMultiplier = 0.6f;
		float m_powderedSugarEffectMultiplier = 10.0f;
		float m_toneMapperEyeExposure = 0.8f;

		// Raymarch settings
		float m_maxRenderDistance = 150000.0f;
		float m_maxHorizontalSampleCount = 192.0f;
		float m_maxVerticalSampleCount = 128.0f;
		bool m_useEarlyExitAtFullOpacity = true;
		bool m_useBayerFilter = true;
		float m_earthRadius = 600000.0f;
		float m_volumetricCloudsStartRadius = 607000.0f;
		float m_volumetricCloudsEndRadius = 633000.0f;
		bool m_renderDirectlyToFullscreen = false;
		bool m_renderActualQuarterResolutionBuffer = false;

		// Wind settings
		glm::vec3 m_windDirection = glm::vec3(1.0f, 0.0f, 0.0f);
		float m_windUpwardBias = 0.15f;
		float m_cloudSpeed = 300.0f;
		float m_cloudTopOffset = 500.0f;

		// Sun settings
		float m_sunIntensity = 0.4f;
		bool m_moveSunManually = true;
		float m_sunPitch = 90.0f;

		glm::vec2 m_resolution;

		std::chrono::system_clock::time_point m_start;

		std::shared_ptr<Prisma::FBO> m_fbo;

		std::shared_ptr<Prisma::Texture> m_textureNoise;

		uint64_t m_worley;

		uint64_t m_perlinWorley;

		uint64_t m_weather;

		void generateNoise();

		void generateWorley();

		void generatePerlinWorley();

		void generateWeather();

		void setVariables();

		int m_downscale = 4;

	};
}