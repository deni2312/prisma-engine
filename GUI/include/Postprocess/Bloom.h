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
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_pingPong;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbPing;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbPong;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texturePing;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texturePong;
};
}