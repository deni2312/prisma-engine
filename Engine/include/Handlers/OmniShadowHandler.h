#pragma once
#include "GlobalData/Platform.h"

#include "GlobalData/InstanceData.h"

#include <glm/glm.hpp>
#include <vector>
#include <Common/interface/RefCntAutoPtr.hpp>

namespace Diligent
{
	struct IPipelineState;
	struct ITexture;
	struct IBuffer;
	struct ITextureView;
	struct IShaderResourceBinding;
}

namespace Prisma {

	class OmniShadowHandler : public InstanceData<OmniShadowHandler> {
	private:


		struct LightPlane {
			glm::vec3 lightPos;
			float far_plane;
		};

		struct OmniShadow
		{
			glm::mat4 shadows[6];
		};

		OmniShadow m_shadows;

		LightPlane m_lightPlane;


		glm::mat4 m_shadowProj;
		std::vector<glm::mat4> m_shadowTransforms;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_lightBuffer;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_shadowBuffer;

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
	public:
		OmniShadowHandler();

		struct OmniShadowData {
			Diligent::RefCntAutoPtr<Diligent::ITexture> depth;
			float nearPlane;
			float farPlane;
			unsigned int width;
			unsigned int height;
			glm::vec3 lightPos;
		};

		void render(OmniShadowData data);
	};
}