#pragma once

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>

namespace Prisma {
class Blit {
public:
    Blit(Diligent::RefCntAutoPtr<Diligent::ITexture> texture);
    void render(Diligent::RefCntAutoPtr<Diligent::ITexture> texture);

private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
};
}