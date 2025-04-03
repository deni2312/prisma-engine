#pragma once

#include <vector>
#include "GenericShadow.h"
#include "GlobalData/Platform.h"
#include "Common/interface/RefCntAutoPtr.hpp"

namespace Diligent
{
	struct ITexture;
	struct IBuffer;
	struct ITextureView;
	struct IShaderResourceBinding;
}

namespace Prisma
{
	class PipelineOmniShadow : public GenericShadow
	{
	public:
		PipelineOmniShadow(unsigned int width, unsigned int height,bool post=false);
		uint64_t id() override;
		float farPlane() override;
		void farPlane(float farPlane) override;
		void init() override;
		float nearPlane() override;
		void nearPlane(float nearPlane) override;

		void update(glm::vec3 lightPos) override;

	private:
		struct LightPlane{
			glm::vec3 lightPos;
			float far_plane;
		};

		struct OmniShadow
		{
			glm::mat4 shadows[6];
		};

		OmniShadow m_shadows;

		LightPlane m_lightPlane;

		unsigned int m_width;
		unsigned int m_height;
		float m_nearPlane = 0.1f;

		unsigned int m_fbo;
		unsigned int m_farPlanePos;
		unsigned int m_lightPos;
		std::vector<unsigned int> m_shadowPosition;

		unsigned int m_farPlanePosAnimation;
		unsigned int m_lightPosAnimation;
		std::vector<unsigned int> m_shadowPositionAnimation;

		uint64_t m_id;
		float m_farPlane = 100.0f;

		glm::mat4 m_shadowProj;
		std::vector<glm::mat4> m_shadowTransforms;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

		Diligent::RefCntAutoPtr<Diligent::ITexture> m_pMSColorRTV;

		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pRTColor[6];

		Diligent::RefCntAutoPtr<Diligent::ITexture> m_depth;

		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_depthView[6];

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_lightBuffer;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_shadowBuffer;
	};
}
