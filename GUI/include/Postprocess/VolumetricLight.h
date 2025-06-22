#pragma once

#include "GlobalData/Platform.h"
#include <Common/interface/RefCntAutoPtr.hpp>

#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Helpers/Blit.h"
#include "glm/glm.hpp"

namespace Prisma::GUI {
class VolumetricLight {
public:
    struct BlurData {
        glm::vec4 fogColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        glm::vec4 maxDistance = glm::vec4(100.0f);
        glm::vec4 stepSize = glm::vec4(1.0f);
        glm::vec4 densityMultiplier = glm::vec4(1.0f);
        glm::vec4 noiseOffset = glm::vec4(0.5f);

        glm::vec4 densityThreshold = glm::vec4(0.1f);
        glm::vec4 noiseTiling = glm::vec4(1.0);

        glm::vec4 lightContribution = glm::vec4(1.0f);

        glm::vec4 lightScattering = glm::vec4(0.2f);
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
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureNoise;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_blur;
    std::unique_ptr<Blit> m_blit;
    BlurData m_blurData;
};
} // namespace Prisma::GUI