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

		void updatePostRender(std::shared_ptr<FBO> fbo = nullptr) override;

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
			float coverageScale = 0.45f;
			float cloudType = 0.8f;
			float lowFrequencyNoiseScale = 0.3f;
			int ignoreDetailNoise = false;

			float highFrequencyNoiseScale = 0.3f;
			float highFrequencyNoiseErodeMuliplier = 0.19f;
			float highFrequencyHeightTransitionMultiplier = 10.0f;
			float anvilBias = 0.1f;


			// Cloud lighting
			glm::vec3 cloudColor = glm::vec3(1.0f, 1.0f, 1.0f);
			float padding1;

			float sunIntensity = 0.4f;
			float ambientColorScale = 0.7f;
			float rainCloudAbsorptionGain = 2.3f;
			float cloudAttenuationScale = 2.5f;

			float phaseEccentricity = 0.5f;
			float phaseSilverLiningIntensity = 0.15f;
			float phaseSilverLiningSpread = 0.5f;
			float coneSpreadMultplier = 0.2f;

			float shadowSampleConeSpreadMultiplier = 0.6f;
			float powderedSugarEffectMultiplier = 10.0f;
			float toneMapperEyeExposure = 0.8f;
			float maxRenderDistance = 150000.0f;

			float maxHorizontalSampleCount = 192.0f;
			float maxVerticalSampleCount = 128.0f;
			int useEarlyExitAtFullOpacity = true;
			int useBayerFilter = true;

			float earthRadius = 600000.0f;
			float volumetricCloudsStartRadius = 607000.0f;
			float volumetricCloudsEndRadius = 633000.0f;
			float padding2;

			// Wind settings
			glm::vec3 windDirection = glm::vec3(1.0f, 0.0f, 0.0f);
			float padding3;

			float windUpwardBias = 0.15f;
			float cloudSpeed = 300.0f;
			float cloudTopOffset = 500.0f;
			float padding4;

			glm::vec2 resolution;
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

	private:
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
