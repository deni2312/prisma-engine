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
#include "SceneObjects/Mesh.h"

namespace Prisma {
class WaterComponent : public RenderComponent {
   public:
    WaterComponent();

    void ui() override;

    void update() override;

    void start() override;

    void destroy() override;

    void updatePreRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void updateTransparentRender(Diligent::RefCntAutoPtr<Diligent::ITexture> accum, Diligent::RefCntAutoPtr<Diligent::ITexture> reveal, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;
   private:
    void createPlaneMesh(float width, int resolution);

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;
    std::function<void(Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>&)> m_updateData;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iBuffer;
    unsigned int m_iBufferSize;
};
}  // namespace Prisma