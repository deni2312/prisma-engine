#pragma once

#include <vector>
#include "GenericShadow.h"
#include "glm/glm.hpp"
#include "../Helpers/Settings.h"
#include "Common/interface/RefCntAutoPtr.hpp"

namespace Diligent
{
	struct ITexture;
}

namespace Prisma
{
	class PipelineCSM : public GenericShadow
	{
	public:
		PipelineCSM(unsigned int width, unsigned int height,bool post=false);
		float farPlane() override;
		void farPlane(float farPlane) override;
		void init() override;
		float nearPlane() override;
		void nearPlane(float nearPlane) override;

		void lightDir(glm::vec3 lightDir);

		glm::vec3 lightDir();

		void update(glm::vec3 lightPos) override;

		std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview);
		Diligent::RefCntAutoPtr<Diligent::ITexture> shadowTexture() override;

		glm::mat4 getLightSpaceMatrix(float nearPlane, float farPlane);

		std::vector<glm::mat4> getLightSpaceMatrices();

		std::vector<float>& cascadeLevels();

		void bias(float bias);

		float bias();

	private:
		unsigned int m_width;

		unsigned int m_height;

		unsigned int m_fbo;

		glm::vec3 m_lightDir = glm::vec3(0.0f);

		uint64_t m_id;

		float m_farPlane = 200.0f;

		float m_nearPlane = 0.1f;

		float m_bias = 0.5f;

		std::vector<float> m_shadowCascadeLevels;

		Settings m_settings;

		bool m_init = false;

		std::vector<glm::mat4> m_lightMatrices;

		Diligent::RefCntAutoPtr<Diligent::ITexture> m_depth;
	};
}
