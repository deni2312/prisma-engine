#pragma once
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>


namespace Prisma {
class Bloom {
public:
    void render();
    Bloom();

private:
    void createShaderBrightness();
    void createShaderPingPong();
    void createShaderRender();

    void renderBrightness();
    void renderPingPong();
    void renderBloom();

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoRender;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbRender;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoBrightness;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbBrightness;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureBrightness;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pingPong;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoPingPong;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbPingPong;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texturePingPong[2];
};
}