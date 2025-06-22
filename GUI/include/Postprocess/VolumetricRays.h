#pragma once

#include <Common/interface/RefCntAutoPtr.hpp>

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Helpers/Blit.h"
#include "glm/glm.hpp"

namespace Prisma::GUI {
class VolumetricRays {
   public:
    struct VolumetricData {
        glm::vec4 exposure = glm::vec4(0.3);
        glm::vec4 decay = glm::vec4(0.95);
        glm::vec4 density = glm::vec4(0.8);
        glm::vec4 weight = glm::vec4(0.5);
        glm::ivec4 samples = glm::ivec4(100);
    };

    void render();
    VolumetricRays();

    void volumetricSettings(VolumetricData VolumetricData);
    VolumetricData volumetricSettings() const;

   private:
    void createShaderVolumetric();
    void createShaderBlit();

    void renderVolumetric();
    void renderBlit();

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoBlit;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbBlit;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoVolumetric;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbVolumetric;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureVolumetric;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureBlit;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureNoise;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_volumetric;
    std::unique_ptr<Blit> m_blit;
    VolumetricData m_volumetricData;
};
}  // namespace Prisma::GUI