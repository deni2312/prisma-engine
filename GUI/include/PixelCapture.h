#pragma once

#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include <string>
#include "SceneObjects/Mesh.h"
#include "GlobalData/InstanceData.h"

namespace Prisma::GUI {
class PixelCapture : public InstanceData<PixelCapture> {
public:
    PixelCapture();
    std::shared_ptr<Mesh> capture(glm::vec2 position, const glm::mat4& model);
    void drawModel(const glm::mat4& model);

private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;


    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoAnimation;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbAnimation;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_pRTDepth;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_pStagingTexture;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_pRTColor;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_mvpVS;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_scalePso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_scaleSrb;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_pRTColorOutput;


    void createDrawPipeline();
    void createDrawAnimationPipeline();
    void createScalePipeline();

    glm::vec4 m_clearColor = glm::vec4(0, 0, 0, 1);
};
}