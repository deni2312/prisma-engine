#pragma once
#include "GlobalData/InstanceData.h"
#include "GlobalData/Platform.h"

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"
#include <glm/glm.hpp>

namespace Prisma {
	class PipelineBlitRT{
	public:
		PipelineBlitRT(Diligent::RefCntAutoPtr<Diligent::ITexture> texture);
		void blit();
	private:
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

	};

}
