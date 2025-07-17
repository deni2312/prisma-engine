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
#include "Helpers/TimeCounter.h"
#include <glm/glm.hpp>

namespace Prisma {
class WaterComponent : public RenderComponent {
   public:
    struct WaterConstants {
        glm::vec4 time=glm::vec4(0.0);
        float waveAmplitude=1;
        float waveFrequency=1;
        float waveSpeed=1;
        int size=1;
        glm::vec3 touchPosition = glm::vec3(0.0f);
        float radius = 0;
    };

    WaterComponent();

    void ui() override;

    void update() override;

    void start() override;

    void destroy() override;

    void updatePreRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void updateTransparentRender(Diligent::RefCntAutoPtr<Diligent::ITexture> accum, Diligent::RefCntAutoPtr<Diligent::ITexture> reveal, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;
   
    void nodePosition(std::shared_ptr<Prisma::Node> node);

    void radius(float radius);

private:
    struct AlignedVertex {
        glm::vec4 position;
        glm::vec4 normal;
        glm::vec4 texCoords;
        glm::vec4 tangent;
        glm::vec4 bitangent;
    };


    void createPlaneMesh();
    void createCompute();
    void computeWater();

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoCompute;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbCompute;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;
    std::function<void(Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>&)> m_updateData;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_constants;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_modelConstant;

    std::shared_ptr<Prisma::Mesh> m_mesh;

    TimeCounter m_counter;

    unsigned int m_iBufferSize;
    int m_resolution = 512;
    float m_length = 100;
    std::function<void()> m_run;

    WaterConstants m_waterConstants;

    std::shared_ptr<Prisma::Node> m_node;
};
}  // namespace Prisma