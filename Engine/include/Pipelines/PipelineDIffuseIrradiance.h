#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../GlobalData/InstanceData.h"
#include "PipelineSkybox.h"

namespace Prisma {
class PipelineDiffuseIrradiance : public InstanceData<PipelineDiffuseIrradiance> {
public:
    void texture(Diligent::RefCntAutoPtr<Diligent::ITexture> texture);

    Diligent::RefCntAutoPtr<Diligent::ITexture> irradianceTexture();

    PipelineDiffuseIrradiance();

private:
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iblData;
    Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pRTColor[6];

    Texture m_texture;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_pMSColorRTV;

    const glm::vec2 m_dimensions = glm::vec2(32, 32);

    const PipelineSkybox::IBLData m_iblTransform;
};
}