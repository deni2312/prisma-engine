#include "Helpers/UpdateTLAS.h"

#include "GlobalData/CacheScene.h"
#include "GlobalData/EngineSettings.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/PrismaFunc.h"
#include "Pipelines/PipelineHandler.h"
#include "engine.h"
#include "Helpers/Logger.h"

// Instance mask.
#define OPAQUE_GEOM_MASK      0x01
#define TRANSPARENT_GEOM_MASK 0x02

// Ray types
#define HIT_GROUP_STRIDE  2
#define PRIMARY_RAY_INDEX 0
#define SHADOW_RAY_INDEX  1

Prisma::UpdateTLAS::UpdateTLAS()
{
    resizeTLAS();
}

void Prisma::UpdateTLAS::update()
{
    // Create or update top-level acceleration structure
    if (Prisma::Engine::getInstance().engineSettings().pipeline == EngineSettings::Pipeline::RAYTRACING) {
        auto meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
        if (!meshes.empty())
        {
            if (Prisma::CacheScene::getInstance().updateSizes()) {
                updateSizeTLAS();
            }

            if (Prisma::CacheScene::getInstance().updateData()) {
                updateTLAS(true);
            }

            if (Prisma::CacheScene::getInstance().updateTextures())
            {
                for (auto update : m_updates)
                {
                    update(m_vertexData, m_primitiveData, m_vertexLocation);
                }
            }
        }
    }
}

void Prisma::UpdateTLAS::resizeTLAS()
{

    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
    auto& meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
    if (!meshes.empty()) {
        if (m_vertexData)
        {
            m_vertexData.Release();
            m_primitiveData.Release();
            m_vertexLocation.Release();
        }

        int sizeVertices = 0;
        int sizePrimitives = 0;
        
        std::vector<UpdateTLAS::VertexBlas> verticesBlas;
        std::vector<glm::ivec4> primitivesBlas;
        std::vector<UpdateTLAS::LocationBlas> locationBlas;

        for (auto mesh : meshes)
        {
            auto& verticesData = mesh->verticesData().vertices;
            auto& indicesData = mesh->verticesData().indices;
            UpdateTLAS::LocationBlas currentLocation;
            currentLocation.location = sizeVertices;
            currentLocation.size = mesh->verticesData().vertices.size();
            currentLocation.locationPrimitive = sizePrimitives;

            locationBlas.push_back(currentLocation);
            for (auto v : verticesData)
            {
                verticesBlas.push_back({ glm::vec4(v.normal,0),glm::vec4(v.texCoords,0,0) });
            }

            for (int i = 0;i < indicesData.size();i += 3)
            {
                primitivesBlas.push_back({ indicesData[i],indicesData[i + 1] ,indicesData[i + 2],0 });
            }

            sizeVertices += verticesData.size();
            sizePrimitives += indicesData.size() / 3;
        }


        Diligent::BufferDesc vertexDataDesc;

        vertexDataDesc.Name = "Vertex Data Buffer";
        vertexDataDesc.Usage = Diligent::USAGE_DEFAULT;
        vertexDataDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        vertexDataDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
        vertexDataDesc.ElementByteStride = sizeof(UpdateTLAS::VertexBlas);
        vertexDataDesc.Size = verticesBlas.size() * sizeof(UpdateTLAS::VertexBlas);
        Diligent::BufferData vertexData;
        vertexData.DataSize = vertexDataDesc.Size;
        vertexData.pData = verticesBlas.data();

        contextData.m_pDevice->CreateBuffer(vertexDataDesc, &vertexData, &m_vertexData);

        Diligent::BufferDesc primitiveDesc;

        primitiveDesc.Name = "Primitive Data Buffer";
        primitiveDesc.Usage = Diligent::USAGE_DEFAULT;
        primitiveDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        primitiveDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
        primitiveDesc.ElementByteStride = sizeof(glm::ivec4);
        primitiveDesc.Size = primitivesBlas.size() * sizeof(glm::ivec4);
        Diligent::BufferData indexData;
        indexData.DataSize = primitiveDesc.Size;
        indexData.pData = primitivesBlas.data();

        contextData.m_pDevice->CreateBuffer(primitiveDesc, &indexData, &m_primitiveData);

        Diligent::BufferDesc vertexLocationDesc;

        vertexLocationDesc.Name = "Vertex Location Buffer";
        vertexLocationDesc.Usage = Diligent::USAGE_DEFAULT;
        vertexLocationDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        vertexLocationDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
        vertexLocationDesc.ElementByteStride = sizeof(UpdateTLAS::LocationBlas);
        vertexLocationDesc.Size = meshes.size() * sizeof(UpdateTLAS::LocationBlas);
        Diligent::BufferData locationData;
        locationData.DataSize = vertexLocationDesc.Size;
        locationData.pData = locationBlas.data();
        contextData.m_pDevice->CreateBuffer(vertexLocationDesc, &locationData, &m_vertexLocation);

        for (auto update : m_updates)
        {
            update(m_vertexData, m_primitiveData, m_vertexLocation);
        }
    }
}

Diligent::RefCntAutoPtr<Diligent::IShaderBindingTable> Prisma::UpdateTLAS::SBT() {
    return m_pSBT;
}

void Prisma::UpdateTLAS::addUpdates(
	std::function<void(Diligent::RefCntAutoPtr<Diligent::IBuffer>, Diligent::RefCntAutoPtr<Diligent::IBuffer>, Diligent::RefCntAutoPtr<Diligent::IBuffer>)> update)
{
    m_updates.push_back(update);
}

void Prisma::UpdateTLAS::updateSizeTLAS() {
    resizeTLAS();
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
    auto meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
    if (meshes.size()>= Define::MAX_RAYTRACING_MESHES)
    {
        Prisma::Logger::getInstance().log(Prisma::LogLevel::ERRORS,"Limit meshes in raytracing pipeline exceeded("+std::to_string(Define::MAX_RAYTRACING_MESHES)+")");
    }

    for (int i = 0;i < meshes.size() && i < Define::MAX_RAYTRACING_MESHES;i++) {
        meshes[i]->uploadBLAS();
    }
    if (m_pTLAS)
    {
        m_ScratchBuffer.Release();
        m_InstanceBuffer.Release();
    }

    // Create TLAS
    if (!m_pTLAS)
    {
        Diligent::TopLevelASDesc TLASDesc;
        TLASDesc.Name = "TLAS";
        TLASDesc.MaxInstanceCount = Define::MAX_RAYTRACING_MESHES;
        TLASDesc.Flags = Diligent::RAYTRACING_BUILD_AS_ALLOW_UPDATE | Diligent::RAYTRACING_BUILD_AS_PREFER_FAST_TRACE;

        contextData.m_pDevice->CreateTLAS(TLASDesc, &m_pTLAS);
        VERIFY_EXPR(m_pTLAS != nullptr);
        Prisma::PipelineHandler::getInstance().raytracing()->srb()->GetVariableByName(Diligent::SHADER_TYPE_RAY_GEN, "g_TLAS")->Set(Prisma::UpdateTLAS::getInstance().TLAS());
        Prisma::PipelineHandler::getInstance().raytracing()->srb()->GetVariableByName(Diligent::SHADER_TYPE_RAY_CLOSEST_HIT, "g_TLAS")->Set(Prisma::UpdateTLAS::getInstance().TLAS());
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

void Prisma::UpdateTLAS::updateTLAS(bool update)
{

    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
    auto meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;

    std::vector<Diligent::TLASBuildInstanceData> instances;

    for (int i = 0; i < meshes.size(); i++)
    {
        Diligent::TLASBuildInstanceData instance;

        instance.InstanceName = meshes[i]->strUUID();
        instance.CustomId = i;
        instance.pBLAS = meshes[i]->blas();
        instance.Mask = OPAQUE_GEOM_MASK;
        glm::mat4 modelMatrix = meshes[i]->parent()->finalMatrix(); // This should include scale, rotation, and translation
        // Assign to the instance transform as a 3x4 row-major matrix
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 4; ++col) {
                instance.Transform.data[row][col] = modelMatrix[col][row]; // glm is column-major, Diligent expects row-major
            }
        }
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
        m_pSBT->BindMissShader("ShadowMiss", SHADOW_RAY_INDEX);

        for (int i = 0;i < meshes.size();i++)
        {
            m_pSBT->BindHitGroupForInstance(m_pTLAS, meshes[i]->strUUID(), PRIMARY_RAY_INDEX, "CubePrimaryHit");
        }
        m_pSBT->BindHitGroupForTLAS(m_pTLAS, SHADOW_RAY_INDEX, nullptr);

        // Update SBT with the shader groups we bound
        contextData.m_pImmediateContext->UpdateSBT(m_pSBT);
    }
}

Diligent::RefCntAutoPtr<Diligent::ITopLevelAS> Prisma::UpdateTLAS::TLAS()
{
    return m_pTLAS;
}
