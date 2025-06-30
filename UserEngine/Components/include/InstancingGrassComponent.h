#pragma once
#include <memory>
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>

#include "Components/RenderComponent.h"
#include "SceneObjects/Mesh.h"

#include "Helpers/PrismaRender.h"
#include "Helpers/TimeCounter.h"



namespace Prisma {
class InstancingGrassComponent : public RenderComponent {
   public:
    InstancingGrassComponent();

    void ui() override;

    void start() override;

    void models(const std::vector<Mesh::MeshData>& models);

    void updateModels(const std::vector<Mesh::MeshData>& models);

    void mesh(std::shared_ptr<Prisma::Mesh> mesh);

    void updatePreRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) override;

    void destroy() override;

   private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbOpaque;
    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;
    std::function<void(Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>&)> m_updateData;
    Prisma::PrismaRender::BufferData m_bufferData;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_modelsBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_timeBuffer;

    std::vector<Mesh::MeshData> m_models;

    std::shared_ptr<Prisma::Mesh> m_mesh;

    TimeCounter m_counter;
};
}  // namespace Prisma