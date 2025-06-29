#pragma once
#include <memory>
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>

#include "Components/RenderComponent.h"

#include "Helpers/PrismaRender.h"


namespace Prisma {
class TreeRendererComponent : public RenderComponent {
   public:
    TreeRendererComponent();

    void ui() override;

    void start() override;

    virtual void updatePreRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth);

    virtual void updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth);

   private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbOpaque;
    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;
    std::function<void(Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>&, Diligent::RefCntAutoPtr<Diligent::IBuffer>&)> m_updateData;
    Prisma::PrismaRender::BufferData m_bufferData;
};
}  // namespace Prisma