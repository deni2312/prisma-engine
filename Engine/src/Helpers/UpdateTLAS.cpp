#include "Helpers/UpdateTLAS.h"

#include "GlobalData/CacheScene.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/PrismaFunc.h"
#include "Pipelines/PipelineHandler.h"

// Instance mask.
#define OPAQUE_GEOM_MASK      0x01
#define TRANSPARENT_GEOM_MASK 0x02

// Ray types
#define HIT_GROUP_STRIDE  1
#define PRIMARY_RAY_INDEX 0

Prisma::UpdateTLAS::UpdateTLAS()
{
}

void Prisma::UpdateTLAS::update()
{
    // Create or update top-level acceleration structure

    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
    auto meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;

    if (Prisma::CacheScene::getInstance().updateSizes() && !meshes.empty()) {
        if (m_pTLAS)
        {
            m_pTLAS.Release();
            m_ScratchBuffer.Release();
            m_InstanceBuffer.Release();
        }

        // Create TLAS
        if (!m_pTLAS)
        {
            Diligent::TopLevelASDesc TLASDesc;
            TLASDesc.Name = "TLAS";
            TLASDesc.MaxInstanceCount = meshes.size();
            TLASDesc.Flags = Diligent::RAYTRACING_BUILD_AS_ALLOW_UPDATE | Diligent::RAYTRACING_BUILD_AS_PREFER_FAST_TRACE;

            contextData.m_pDevice->CreateTLAS(TLASDesc, &m_pTLAS);
            VERIFY_EXPR(m_pTLAS != nullptr);

        }

        // Create scratch buffer
        if (!m_ScratchBuffer)
        {
            Diligent::BufferDesc BuffDesc;
            BuffDesc.Name = "TLAS Scratch Buffer";
            BuffDesc.Usage = Diligent::USAGE_DEFAULT;
            BuffDesc.BindFlags = Diligent::BIND_RAY_TRACING;
            BuffDesc.Size = std::max(m_pTLAS->GetScratchBufferSizes().Build, m_pTLAS->GetScratchBufferSizes().Update);

            contextData.m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_ScratchBuffer);
            VERIFY_EXPR(m_ScratchBuffer != nullptr);
        }

        // Create instance buffer
        if (!m_InstanceBuffer)
        {
            Diligent::BufferDesc BuffDesc;
            BuffDesc.Name = "TLAS Instance Buffer";
            BuffDesc.Usage = Diligent::USAGE_DEFAULT;
            BuffDesc.BindFlags = Diligent::BIND_RAY_TRACING;
            BuffDesc.Size = Diligent::TLAS_INSTANCE_DATA_SIZE * meshes.size();

            contextData.m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_InstanceBuffer);
            VERIFY_EXPR(m_InstanceBuffer != nullptr);
        }
        updateTLAS(false);
    }

    if (Prisma::CacheScene::getInstance().updateData() && !meshes.empty()) {
        updateTLAS(true);
    }
}

Diligent::RefCntAutoPtr<Diligent::IShaderBindingTable> Prisma::UpdateTLAS::SBT() {
    return m_pSBT;
}

void Prisma::UpdateTLAS::updateTLAS(bool update)
{

    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
    auto meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
    std::vector<Diligent::TLASBuildInstanceData> instances;
    for (int i = 0;i < meshes.size();i++)
    {
        // Setup instances
        Diligent::TLASBuildInstanceData instance;
        std::string index = std::to_string(i);
        instance.InstanceName = "1";
        instance.CustomId = i; // texture index
        instance.pBLAS = meshes[i]->blas();
        instance.Mask = OPAQUE_GEOM_MASK;

        glm::vec3 translation = meshes[i]->parent()->finalMatrix()[3];

        instance.Transform.SetTranslation(translation.x, translation.y, translation.z);
        instances.push_back(instance);
    }
    // Build or update TLAS
    Diligent::BuildTLASAttribs Attribs;
    Attribs.pTLAS = m_pTLAS;
    Attribs.Update = update;

    // Scratch buffer will be used to store temporary data during TLAS build or update.
    // Previous content in the scratch buffer will be discarded.
    Attribs.pScratchBuffer = m_ScratchBuffer;

    // Instance buffer will store instance data during TLAS build or update.
    // Previous content in the instance buffer will be discarded.
    Attribs.pInstanceBuffer = m_InstanceBuffer;

    // Instances will be converted to the format that is required by the graphics driver and copied to the instance buffer.
    Attribs.pInstances = instances.data();
    Attribs.InstanceCount = instances.size();

    // Bind hit shaders per instance, it allows you to change the number of geometries in BLAS without invalidating the shader binding table.
    Attribs.BindingMode = Diligent::HIT_GROUP_BINDING_MODE_PER_INSTANCE;
    Attribs.HitGroupStride = HIT_GROUP_STRIDE;

    // Allow engine to change resource states.
    Attribs.TLASTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
    Attribs.BLASTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
    Attribs.InstanceBufferTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
    Attribs.ScratchBufferTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

    contextData.m_pImmediateContext->BuildTLAS(Attribs);

    // Create shader binding table.

    Diligent::ShaderBindingTableDesc SBTDesc;
    SBTDesc.Name = "SBT";
    SBTDesc.pPSO = Prisma::PipelineHandler::getInstance().raytracing()->pso();

    if (!update) {

        if (m_pSBT)
        {
            m_pSBT.Release();
        }
        contextData.m_pDevice->CreateSBT(SBTDesc, &m_pSBT);
        VERIFY_EXPR(m_pSBT != nullptr);
        m_pSBT->BindRayGenShader("Main");

        m_pSBT->BindMissShader("PrimaryMiss", PRIMARY_RAY_INDEX);
        for (int i = 0;i < meshes.size();i++)
        {
            std::string index = std::to_string(i);
            m_pSBT->BindHitGroupForInstance(m_pTLAS, "1", PRIMARY_RAY_INDEX, "CubePrimaryHit");

        }
        // Hit groups for primary ray
        // clang-format off

        // Update SBT with the shader groups we bound
        contextData.m_pImmediateContext->UpdateSBT(m_pSBT);
    }
}

Diligent::RefCntAutoPtr<Diligent::ITopLevelAS> Prisma::UpdateTLAS::TLAS()
{
    return m_pTLAS;
}
