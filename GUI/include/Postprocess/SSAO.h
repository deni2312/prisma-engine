#pragma once
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>

#include "Helpers/Blit.h"
#include <Common/interface/RefCntAutoPtr.hpp>
#include "Postprocess/PostprocessEffect.h"
#include "Helpers/Settings.h"

namespace Prisma::GUI {
class SSAO : public Prisma::PostprocessEffect {
   public:
    void render() override;
    SSAO();

    void apply(bool apply);

    bool apply();

   private:
    bool m_apply = false;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_buffer;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_noiseTexture;

    std::unique_ptr<Prisma::Blit> m_blit;
    float ourLerp(float a, float b, float f);
};
}  // namespace Prisma::GUI