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

   private:
    unsigned int m_width;
    unsigned int m_height;

    void create();

    Diligent::SAMPLE_COUNT m_SupportedSampleCounts;
    int m_SampleCount;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pMSColorRTV;

    Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pMSDepthDSV;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbTransparent;

    std::function<void(Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>&, Diligent::RefCntAutoPtr<Diligent::IBuffer>&)> m_updateData;
};
}  // namespace Prisma