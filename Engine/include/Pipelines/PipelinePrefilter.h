#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "PipelineFullScreen.h"
#include <memory>

#include "PipelineSkybox.h"
#include "../GlobalData/InstanceData.h"

namespace Prisma {
class PipelinePrefilter : public InstanceData<PipelinePrefilter> {
public:
        void texture(Diligent::RefCntAutoPtr<Diligent::ITexture> texture);

        Diligent::RefCntAutoPtr<Diligent::ITexture> prefilterTexture();

        PipelinePrefilter();

private:
        struct RoughnessResolution {
                float roughness;
                int resolution;
                glm::vec2 padding;
        };

        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iblData;
        Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iblResolution;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pRTColor[6];

        void calculateSkybox();

        Texture m_texture;

        Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

        Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

        Diligent::RefCntAutoPtr<Diligent::ITexture> m_pMSColorRTV;

        const glm::vec2 m_dimensions = glm::vec2(128, 128);

        const PipelineSkybox::IBLData m_iblTransform;
};
}