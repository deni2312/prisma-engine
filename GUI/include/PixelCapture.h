#pragma once

#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include <string>
#include "../../Engine/include/SceneObjects/Mesh.h"
#include "../../Engine/include/GlobalData/InstanceData.h"

namespace Prisma
{
	class PixelCapture : public InstanceData<PixelCapture>
	{
	public:
		PixelCapture();
		std::shared_ptr<Mesh> capture(glm::vec2 position, const glm::mat4& model);

	private:
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pRTColor;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pRTDepth;

	};
}
