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

namespace Prisma
{
	class CloudComponent : public Component
	{
	public:
		CloudComponent();

		void ui() override;

		void updateRender(std::shared_ptr<FBO> fbo = nullptr) override;

		void start() override;

	private:
		unsigned int m_modelPos;

		unsigned int m_lightPos;

		unsigned int m_timePos;

		unsigned int m_noisePos;

		unsigned int m_invViewPos;
		unsigned int m_invProjPos;
		unsigned int m_camPos;

		unsigned int m_perlworlPos;
		unsigned int m_worlPos;

		unsigned int m_upscalePos;
		unsigned int m_resUpscalePos;
		unsigned int m_factorPos;

		GLuint m_downscalesq = 4 * 4;

		std::shared_ptr<SSBO> m_ssbo;

		struct alignas(16) CloudSSBO
		{
			// Cloud model
			float m_coverageScale = 0.45f;
			float m_cloudType = 0.8f;
			float m_lowFrequencyNoiseScale = 0.3f;
			int m_ignoreDetailNoise = false;

			float m_highFrequencyNoiseScale = 0.3f;
			float m_highFrequencyNoiseErodeMuliplier = 0.19f;
			float m_highFrequencyHeightTransitionMultiplier = 10.0f;
			float m_anvilBias = 0.1f;


			// Cloud lighting
			glm::vec3 m_cloudColor = glm::vec3(1.0f, 1.0f, 1.0f);
			float padding1;

			float m_sunIntensity = 0.4f;
			float m_ambientColorScale = 0.7f;
			float m_rainCloudAbsorptionGain = 2.3f;
			float m_cloudAttenuationScale = 2.5f;

			float m_phaseEccentricity = 0.5f;
			float m_phaseSilverLiningIntensity = 0.15f;
			float m_phaseSilverLiningSpread = 0.5f;
			float m_coneSpreadMultplier = 0.2f;

			float m_shadowSampleConeSpreadMultiplier = 0.6f;
			float m_powderedSugarEffectMultiplier = 10.0f;
			float m_toneMapperEyeExposure = 0.8f;
			float m_maxRenderDistance = 150000.0f;

			float m_maxHorizontalSampleCount = 192.0f;
			float m_maxVerticalSampleCount = 128.0f;
			int m_useEarlyExitAtFullOpacity = true;
			int m_useBayerFilter = true;

			float m_earthRadius = 600000.0f;
			float m_volumetricCloudsStartRadius = 607000.0f;
			float m_volumetricCloudsEndRadius = 633000.0f;
			float padding2;

			// Wind settings
			glm::vec3 m_windDirection = glm::vec3(1.0f, 0.0f, 0.0f);
			float padding3;

			float m_windUpwardBias = 0.15f;
			float m_cloudSpeed = 300.0f;
			float m_cloudTopOffset = 500.0f;
			float padding4;

			glm::vec2 m_resolution;
			glm::vec2 padding5;
		};

		CloudSSBO m_cloudSSBO;

		std::chrono::system_clock::time_point m_start;

		std::shared_ptr<FBO> m_fbo;

		uint64_t m_worley;

		uint64_t m_perlinWorley;

		uint64_t m_weather;

		void generateNoise();

		void generateWorley();

		void generatePerlinWorley();

		void generateWeather();

		void setVariables();

		const int m_downscale = 4;

		std::function<void()> m_startButton;

		std::function<void()> m_updateButton;

		Settings m_settings;

		std::shared_ptr<Shader> m_cloudShader;
		std::shared_ptr<Shader> m_noiseShader;
		std::shared_ptr<Shader> m_worleyShader;
		std::shared_ptr<Shader> m_perlinWorleyShader;
		std::shared_ptr<Shader> m_weatherShader;
		std::shared_ptr<Shader> m_upscaleShader;
	};
}
