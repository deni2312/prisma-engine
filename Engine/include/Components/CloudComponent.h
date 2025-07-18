#pragma once

#include <Common/interface/RefCntAutoPtr.hpp>

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Helpers/Blit.h"
#include "Helpers/Settings.h"
#include "Helpers/TimeCounter.h"
#include "RenderComponent.h"

namespace Prisma {
class CloudComponent : public RenderComponent {
   public:
    enum class CLOUD_TYPE { BOX, SPHERE, DONUT };

    CloudComponent();

    void ui() override;

    void update() override;

    void start() override;

    void destroy() override;

    void updatePreRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void updateTransparentRender(Diligent::RefCntAutoPtr<Diligent::ITexture> accum, Diligent::RefCntAutoPtr<Diligent::ITexture> reveal, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void cloudType(CLOUD_TYPE type);

    CLOUD_TYPE cloudType();

   private:
    void createCloud();

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_cloudConstants;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
    Settings m_settings;

    std::function<void()> m_run;

    TimeCounter m_counter;

    struct CloudConstants {
        glm::vec4 resolution;
        glm::vec4 cloudPosition = glm::vec4(0);  // This uniform is available but not used in the provided SDF.
        glm::vec4 dirLight = glm::vec4(0, 1, 0, 0);
        glm::vec4 color = glm::vec4(1.0);
        glm::ivec4 type = glm::ivec4(0);
        float time = 0;
        int maxSteps = 64;
        float marchSize = 0.01;
        float maxDistance = 50.0;
    };
    CloudConstants m_constants;

    CLOUD_TYPE m_type = CLOUD_TYPE::BOX;

    ComponentList m_status;
};
} 