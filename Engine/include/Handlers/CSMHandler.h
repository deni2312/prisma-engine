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

	class CSMHandler : public InstanceData<CSMHandler> {
	private:

		struct CSMShadow
		{
			glm::mat4 shadows[16];
		};

		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_lightBuffer;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_shadowBuffer;

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
	public:
		CSMHandler();

		struct CSMData {
			Diligent::RefCntAutoPtr<Diligent::ITexture> depth;
			CSMShadow shadows;
		};

		void render(CSMData& data);
	};
}