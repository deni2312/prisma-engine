#pragma once
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>


namespace Prisma::GUI {
class PostprocessingStyles {
public:
    enum class EFFECTS { NORMAL, SEPPIA, CARTOON, VIGNETTE, BLOOM };

    void render(EFFECTS effect);
    PostprocessingStyles();

private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoBlit;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbBlit;

    void createShaderEffects();
    void createShaderBlit();

    void renderEffects(EFFECTS effect);
    void renderBlit();

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_current;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
};
}