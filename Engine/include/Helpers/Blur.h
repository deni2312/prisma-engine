#pragma once

#include <Common/interface/RefCntAutoPtr.hpp>

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include <array>

namespace Prisma {
class Blur {
   public:
    Blur(Diligent::RefCntAutoPtr<Diligent::ITexture> texture);
    void render(unsigned int amount=10);
    std::array<Diligent::RefCntAutoPtr<Diligent::ITexture>,2> pingPong();


   private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_buffer;
    std::array<Diligent::RefCntAutoPtr<Diligent::ITexture>, 2> m_texturePingPong;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
};
}  // namespace Prisma