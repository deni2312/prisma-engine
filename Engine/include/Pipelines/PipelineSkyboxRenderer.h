#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include <memory>
#include "../GlobalData/InstanceData.h"

namespace Prisma {
class PipelineSkyboxRenderer {
public:
    void render();
    void texture(Diligent::RefCntAutoPtr<Diligent::ITexture> texture);
    PipelineSkyboxRenderer();

private:
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_mvpVS;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
};
}