#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include <memory>
#include "../GlobalData/InstanceData.h"

namespace Prisma
{
	class PipelineLUT : public InstanceData<PipelineLUT>
	{
	public:
		void texture();

		PipelineLUT();

	private:
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_shader;

		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pMSColorRTV;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pMSDepthDSV;
	};
}
