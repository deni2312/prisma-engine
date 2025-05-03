#pragma once
#include "GlobalData/InstanceData.h"
#include "GlobalData/Platform.h"

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"
#include <glm/glm.hpp>

namespace Prisma::GUI {
class ScenePipeline : public InstanceData<ScenePipeline> {
public:
    ScenePipeline();
    void render(glm::mat4 model, Diligent::ITextureView* color, Diligent::ITextureView* depth);

private:
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_mvpVS;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
};
}