#pragma once
#include <Common/interface/RefCntAutoPtr.hpp>

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Helpers/Blit.h"
#include "glm/glm.hpp"

namespace Prisma::GUI {
class VolumetricLight {
public:
    struct BlurData {
        float exposure = 0.3;
        float decay = 0.95;
        float density = 0.8;
        float weight = 0.01;
        glm::ivec4 numSamples = glm::vec4(100);
    };

    void render();
    VolumetricLight();

    void blurData(BlurData blurData);
    BlurData blurData() const;

private:
    void createShaderRender();
    void createShaderBlur();
    void createShaderBlit();

    void renderData();
    void renderBlur();
    void renderBlit();

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoBlit;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbBlit;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoBlur;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbBlur;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureDepth;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureBlur;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureBlit;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_blur;
    std::unique_ptr<Blit> m_blit;
    BlurData m_blurData;
};
} // namespace Prisma::GUI