#pragma once
#include "../GlobalData/Platform.h"
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>

#include "Helpers/Blit.h"

namespace Prisma {
class PipelineSoftwareRT {
public:
    PipelineSoftwareRT(unsigned int width, unsigned int height);
    void render();

private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_rtData;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
    std::unique_ptr<Blit> m_blit;
    unsigned int m_width;
    unsigned int m_height;
};
}