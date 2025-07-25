#pragma once

#include <memory>

#include "../Handlers/MeshHandler.h"
#include "../Helpers/Blit.h"
#include "../Helpers/Settings.h"
#include "../SceneData/MeshIndirect.h"
#include "../SceneData/SceneLoader.h"
#include "../SceneObjects/Camera.h"
#include "PipelineForwardTransparent.h"
#include "PipelineFullScreen.h"
#include "PipelinePrePass.h"
#include "PipelineDeferred.h"
#include "PipelineSSR.h"

namespace Prisma {
class PipelineDeferredForward {
   public:
    PipelineDeferredForward(const unsigned int& width = 1920, const unsigned int& height = 1080);
    void render();
    ~PipelineDeferredForward();
    Diligent::RefCntAutoPtr<Diligent::ITexture> positionTexture();
    Diligent::RefCntAutoPtr<Diligent::ITexture> normalTexture();
    Diligent::RefCntAutoPtr<Diligent::ITexture> albedoTexture();
   private:
    unsigned int m_width;
    unsigned int m_height;

    void create();
    void createAnimation();

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbOpaque;

    std::function<void(Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>&, Diligent::RefCntAutoPtr<Diligent::IBuffer>&)> m_updateData;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoAnimation;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignatureAnimation;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbAnimation;

    std::function<void()> m_updateDataAnimation;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_positionTexture;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_normalTexture;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_albedoTexture;

    std::unique_ptr<PipelineDeferred> m_deferredPipeline;

    std::unique_ptr<PipelineSSR> m_ssr;
};
}  // namespace Prisma