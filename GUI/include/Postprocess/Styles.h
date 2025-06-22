#pragma once
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>

#include "Helpers/Blit.h"


namespace Prisma::GUI {
class PostprocessingStyles {
public:
    enum class EFFECTS { NORMAL, SEPPIA, CARTOON, VIGNETTE, BLOOM, VOLUMETRIC,RAYS };

    void render(EFFECTS effect);
    PostprocessingStyles();

private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    void createShaderEffects();
    void renderEffects(EFFECTS effect);
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_current;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;

    std::unique_ptr<Blit> m_blit;
};
}