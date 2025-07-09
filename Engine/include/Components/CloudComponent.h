#pragma once

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>
#include "Helpers/Blit.h"
#include "RenderComponent.h"
#include "Helpers/Settings.h"
#include "Helpers/TimeCounter.h"

namespace Prisma {
class CloudComponent : public RenderComponent {
   public:
    CloudComponent();

    void ui() override;

    void update() override;

    void start() override;

    void destroy() override;

    void updatePreRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void updateTransparentRender(Diligent::RefCntAutoPtr<Diligent::ITexture> accum, Diligent::RefCntAutoPtr<Diligent::ITexture> reveal, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

   private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
    std::unique_ptr<Blit> m_blit;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_cloudTexture;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_cloudConstants;
    Settings m_settings;

    TimeCounter m_counter;

    struct CloudConstants {
        glm::vec3 resolution;
        float time;
        glm::vec4 cloudPosition;
    };
};
}