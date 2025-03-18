#pragma once
#include "../../Engine/include/GlobalData/InstanceData.h"
#include "../../Engine/include/GlobalData/Platform.h"

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"


namespace Prisma {
	class ScenePipeline : public Prisma::InstanceData<ScenePipeline>{
	public:
		ScenePipeline();
		void render();
	private:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_mvpVS;
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_shader;
	};

}
