#pragma once

#include <memory>

#include "../Handlers/MeshHandler.h"
#include "../Helpers/Blit.h"
#include "../Helpers/Settings.h"
#include "../SceneData/MeshIndirect.h"
#include "../SceneData/SceneLoader.h"
#include "../SceneObjects/Camera.h"
#include "PipelineFullScreen.h"
#include "PipelinePrePass.h"

namespace Prisma {
class PipelineDeferred {
   public:
    PipelineDeferred(Diligent::RefCntAutoPtr<Diligent::ITexture> albedo, Diligent::RefCntAutoPtr<Diligent::ITexture> normal, Diligent::RefCntAutoPtr<Diligent::ITexture> position);
    void render();
    ~PipelineDeferred();

   private:

    void create();

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbOpaque;

    std::function<void(Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>&, Diligent::RefCntAutoPtr<Diligent::IBuffer>&)> m_updateData;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignatureAnimation;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_positionTexture;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_normalTexture;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_albedoTexture;
};
}  // namespace Prisma