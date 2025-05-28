#pragma once

#include <memory>

#include "../Handlers/MeshHandler.h"
#include "../Helpers/Settings.h"
#include "../SceneData/MeshIndirect.h"
#include "../SceneData/SceneLoader.h"
#include "../SceneObjects/Camera.h"
#include "PipelineFullScreen.h"
#include "PipelinePrePass.h"

namespace Prisma {
class PipelineForwardTransparent {
   public:
    PipelineForwardTransparent(const unsigned int& width = 1920, const unsigned int& height = 1080);
    void render();
    ~PipelineForwardTransparent();

    Diligent::RefCntAutoPtr<Diligent::ITexture> accum();
    Diligent::RefCntAutoPtr<Diligent::ITexture> reveal();

   private:
    unsigned int m_width;
    unsigned int m_height;

    void create();

    Diligent::SAMPLE_COUNT m_SupportedSampleCounts;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_accum;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_reveal;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbTransparent;

    std::function<void(Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>&, Diligent::RefCntAutoPtr<Diligent::IBuffer>&)> m_updateData;
};
}  // namespace Prisma