#pragma once

#include <Common/interface/RefCntAutoPtr.hpp>

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Helpers/Blit.h"
#include "PostprocessEffect.h"
#include "glm/glm.hpp"

namespace Prisma {
class CloudPostprocess : public PostprocessEffect {
   public:
    CloudPostprocess();

    void render() override;

   private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
    std::unique_ptr<Blit> m_blit;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_cloudTexture;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_cloudConstants;

    struct CloudConstants {
        glm::vec4 resolution;
    };
};
}  // namespace Prisma