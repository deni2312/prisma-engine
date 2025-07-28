#pragma once
#include <Common/interface/RefCntAutoPtr.hpp>

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Helpers/Blit.h"
#include "Helpers/Blur.h"
#include "Helpers/Settings.h"

namespace Prisma {
class PipelineSSAO {
   public:
    void render();
    PipelineSSAO(Diligent::RefCntAutoPtr<Diligent::ITexture> normal, Diligent::RefCntAutoPtr<Diligent::ITexture> position);

    Diligent::RefCntAutoPtr<Diligent::ITexture> ssaoTexture();


   private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_buffer;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_noiseTexture;

    std::unique_ptr<Prisma::Blur> m_blur;

    float ourLerp(float a, float b, float f);
};
}  // namespace Prisma::GUI