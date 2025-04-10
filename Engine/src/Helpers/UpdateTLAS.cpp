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

bool updateTLAS = false;

void Prisma::UpdateTLAS::update()
{
    // Create or update top-level acceleration structure

    static constexpr int NumInstances = 1;

    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    if (updateTLAS) {
        auto mesh = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes[0];

        // Setup instances
        Diligent::TLASBuildInstanceData Instances[NumInstances] = {};

        Instances[0].InstanceName = "Cube Instance 1";
        Instances[0].pBLAS = mesh->blas();
        Instances[0].Mask = OPAQUE_GEOM_MASK;
        Instances[0].Transform.SetTranslation(0.0f, 0.0f, 0.0f);


        // Build or update TLAS
        Diligent::BuildTLASAttribs Attribs;
        Attribs.pTLAS = m_pTLAS;
        Attribs.Update = updateTLAS;

        // Scratch buffer will be used to store temporary data during TLAS build or update.
        // Previous content in the scratch buffer will be discarded.
        Attribs.pScratchBuffer = m_ScratchBuffer;

        // Instance buffer will store instance data during TLAS build or update.
        // Previous content in the instance buffer will be discarded.
        Attribs.pInstanceBuffer = m_InstanceBuffer;

        // Instances will be converted to the format that is required by the graphics driver and copied to the instance buffer.
        Attribs.pInstances = Instances;
        Attribs.InstanceCount = _countof(Instances);

        // Bind hit shaders per instance, it allows you to change the number of geometries in BLAS without invalidating the shader binding table.
        Attribs.BindingMode = Diligent::HIT_GROUP_BINDING_MODE_PER_INSTANCE;
        Attribs.HitGroupStride = HIT_GROUP_STRIDE;

        // Allow engine to change resource states.
        Attribs.TLASTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        Attribs.BLASTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        Attribs.InstanceBufferTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        Attribs.ScratchBufferTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        contextData.m_pImmediateContext->BuildTLAS(Attribs);
    }

    if (Prisma::CacheScene::getInstance().updateSizes() && Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size() > 0) {
        auto mesh = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes[0];
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
            TLASDesc.MaxInstanceCount = NumInstances;
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
            BuffDesc.Size = Diligent::TLAS_INSTANCE_DATA_SIZE * NumInstances;

            contextData.m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_InstanceBuffer);
            VERIFY_EXPR(m_InstanceBuffer != nullptr);
        }
        // Setup instances
        Diligent::TLASBuildInstanceData Instances[NumInstances] = {};
        Instances[0].InstanceName = "Cube Instance 1";
        Instances[0].CustomId = 0; // texture index
        Instances[0].pBLAS = mesh->blas();
        Instances[0].Mask = OPAQUE_GEOM_MASK;
        Instances[0].Transform.SetTranslation(0.0f, 0.0f, 0.0f);


        // Build or update TLAS
        Diligent::BuildTLASAttribs Attribs;
        Attribs.pTLAS = m_pTLAS;
        Attribs.Update = updateTLAS;

        // Scratch buffer will be used to store temporary data during TLAS build or update.
        // Previous content in the scratch buffer will be discarded.
        Attribs.pScratchBuffer = m_ScratchBuffer;

        // Instance buffer will store instance data during TLAS build or update.
        // Previous content in the instance buffer will be discarded.
        Attribs.pInstanceBuffer = m_InstanceBuffer;

        // Instances will be converted to the format that is required by the graphics driver and copied to the instance buffer.
        Attribs.pInstances = Instances;
        Attribs.InstanceCount = _countof(Instances);

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

        contextData.m_pDevice->CreateSBT(SBTDesc, &m_pSBT);
        VERIFY_EXPR(m_pSBT != nullptr);
        m_pSBT->BindRayGenShader("Main");

        m_pSBT->BindMissShader("PrimaryMiss", PRIMARY_RAY_INDEX);

        // Hit groups for primary ray
        // clang-format off
        m_pSBT->BindHitGroupForInstance(m_pTLAS, "Cube Instance 1", PRIMARY_RAY_INDEX, "CubePrimaryHit");

        // Update SBT with the shader groups we bound
        contextData.m_pImmediateContext->UpdateSBT(m_pSBT);
        updateTLAS = true;
    }
}

Diligent::RefCntAutoPtr<Diligent::IShaderBindingTable> Prisma::UpdateTLAS::SBT() {
    return m_pSBT;
}

Diligent::RefCntAutoPtr<Diligent::ITopLevelAS> Prisma::UpdateTLAS::TLAS()
{
    return m_pTLAS;
}
