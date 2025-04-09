#include "Helpers/UpdateTLAS.h"

#include "TopLevelAS.h"
#include "GlobalData/CacheScene.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/PrismaFunc.h"
// Instance mask.
#define OPAQUE_GEOM_MASK      0x01
#define TRANSPARENT_GEOM_MASK 0x02

// Ray types
#define HIT_GROUP_STRIDE  2
#define PRIMARY_RAY_INDEX 0
#define SHADOW_RAY_INDEX  1

Prisma::UpdateTLAS::UpdateTLAS()
{
}

void Prisma::UpdateTLAS::update()
{
    // Create or update top-level acceleration structure

    static constexpr int NumInstances = 1;


    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
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

            //m_pRayTracingSRB->GetVariableByName(Diligent::SHADER_TYPE_RAY_GEN, "g_TLAS")->Set(m_pTLAS);
            //m_pRayTracingSRB->GetVariableByName(Diligent::SHADER_TYPE_RAY_CLOSEST_HIT, "g_TLAS")->Set(m_pTLAS);
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

        Instances[0].InstanceName = "Ground Instance";
        Instances[0].pBLAS = mesh->blas();
        Instances[0].Mask = OPAQUE_GEOM_MASK;
        Instances[0].Transform.SetTranslation(0.0f, 0.0f, 0.0f);


        // Build or update TLAS
        Diligent::BuildTLASAttribs Attribs;
        Attribs.pTLAS = m_pTLAS;
        Attribs.Update = false;

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
}
