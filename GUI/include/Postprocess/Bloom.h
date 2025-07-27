#pragma once
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>

#include "Helpers/Blit.h"
#include <Helpers/Blur.h>


namespace Prisma::GUI {
class Bloom {
public:
    void render();
    Bloom();

private:
    void createShaderBrightness();
    void createShaderRender();

    void renderBrightness();
    void renderBloom();

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoRender;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbRender;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoBrightness;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbBrightness;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureBrightness;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureBlit;
    std::unique_ptr<Blit> m_blit;
    std::unique_ptr<Blur> m_blur;
};
}