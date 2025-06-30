#include "Pipelines/PipelineSoftwareRT.h"

#include "GlobalData/GlobalShaderNames.h"
#include "GlobalData/PrismaFunc.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Pipelines/PipelineHandler.h"
#include <Helpers/BVHHelper.h>

#include "engine.h"
#include "Helpers/BVHHelper.h"
#include "Helpers/BVHHelper.h"
#include "Helpers/BVHHelper.h"
#include "Helpers/BVHHelper.h"
#include "Helpers/BVHHelper.h"

Prisma::PipelineSoftwareRT::PipelineSoftwareRT(unsigned int width, unsigned int height): m_width{width}, m_height{height} {
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;

    // Create a shader source stream factory to load shaders from files.
    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    contextData.engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

    Diligent::RefCntAutoPtr<Diligent::IShader> pResetParticleListsCS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_COMPUTE;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Software RT CS";
        ShaderCI.FilePath = "../../../Engine/Shaders/SoftwareRTPipeline/compute.glsl";
        contextData.device->CreateShader(ShaderCI, &pResetParticleListsCS);
    }

    Diligent::ComputePipelineStateCreateInfo PSOCreateInfo;
    Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

    // This is a compute pipeline
    PSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;

    PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    RTColorDesc.Width = contextData.swapChain->GetDesc().Width;
    RTColorDesc.Height = contextData.swapChain->GetDesc().Height;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
    // The render target can be bound as a shader resource and as a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET | Diligent::BIND_UNORDERED_ACCESS;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.ClearValue.Color[0] = 0.350f;
    RTColorDesc.ClearValue.Color[1] = 0.350f;
    RTColorDesc.ClearValue.Color[2] = 0.350f;
    RTColorDesc.ClearValue.Color[3] = 1.f;
    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_texture);

    // clang-format off
    /*	Diligent::ShaderResourceVariableDesc Vars[] =
            {
    
            };
        // clang-format on
        PSODesc.ResourceLayout.Variables = Vars;
        PSODesc.ResourceLayout.NumVariables = _countof(Vars);
        */
    std::string samplerRepeatName = "textureRepeat_sampler";

    Diligent::PipelineResourceDesc Resources[] =
    {
        {Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {Diligent::SHADER_TYPE_COMPUTE, ShaderNames::MUTABLE_MODELS.c_str(), 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_COMPUTE, "TotalSizes", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {Diligent::SHADER_TYPE_COMPUTE, "SizeData", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_COMPUTE, "vertices", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_COMPUTE, "indices", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        //{Diligent::SHADER_TYPE_COMPUTE, "verticesBVH", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        //{Diligent::SHADER_TYPE_COMPUTE, "nodesBVH", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_COMPUTE, "screenTexture", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {Diligent::SHADER_TYPE_COMPUTE, ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str(), Define::MAX_MESHES, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {Diligent::SHADER_TYPE_COMPUTE, samplerRepeatName.c_str(), 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
    };

    Diligent::PipelineResourceSignatureDesc ResourceSignDesc;
    ResourceSignDesc.NumResources = _countof(Resources);
    ResourceSignDesc.Resources = Resources;

    PSODesc.Name = "Software RT";
    PSOCreateInfo.pCS = pResetParticleListsCS;

    contextData.device->CreatePipelineResourceSignature(ResourceSignDesc, &m_pResourceSignature);

    Diligent::IPipelineResourceSignature* ppSignatures[]{m_pResourceSignature};

    PSOCreateInfo.ppResourceSignatures = ppSignatures;
    PSOCreateInfo.ResourceSignaturesCount = _countof(ppSignatures);
    contextData.device->CreateComputePipelineState(PSOCreateInfo, &m_pso);

    Diligent::BufferDesc CBDesc;
    CBDesc.Name = "Sizes";
    CBDesc.Size = sizeof(Sizes);
    CBDesc.Usage = Diligent::USAGE_DEFAULT;
    CBDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS;
    CBDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    CBDesc.ElementByteStride = sizeof(Sizes);
    CBDesc.Size = sizeof(Sizes);
    contextData.device->CreateBuffer(CBDesc, nullptr, &m_size);

    Diligent::BufferDesc CBDescTotal;
    CBDescTotal.Name = "Total Sizes";
    CBDescTotal.Size = sizeof(glm::ivec4);
    CBDescTotal.Usage = Diligent::USAGE_DEFAULT;
    CBDescTotal.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    CBDescTotal.Size = sizeof(glm::ivec4);
    contextData.device->CreateBuffer(CBDescTotal, nullptr, &m_totalMeshes);

    Diligent::SamplerDesc SamLinearRepeatDesc
    {
        Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
        Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP
    };
    Diligent::RefCntAutoPtr<Diligent::ISampler> samplerRepeat;

    contextData.device->CreateSampler(SamLinearRepeatDesc, &samplerRepeat);
    Diligent::IDeviceObject* samplerDeviceRepeat = samplerRepeat;

    m_pResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, samplerRepeatName.c_str())->Set(samplerDeviceRepeat);
    m_pResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "screenTexture")->Set(m_texture->GetDefaultView(Diligent::TEXTURE_VIEW_UNORDERED_ACCESS));
    m_pResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());
    m_pResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "TotalSizes")->Set(m_totalMeshes);

    m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);
    m_blitRT = std::make_unique<PipelineBlitRT>(m_texture);
    Diligent::BufferDesc RTBufferDescVertex;
    RTBufferDescVertex.Name = "Vertices Buffer";
    RTBufferDescVertex.Usage = Diligent::USAGE_DEFAULT;
    RTBufferDescVertex.BindFlags = Diligent::BIND_UNORDERED_ACCESS;
    RTBufferDescVertex.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    RTBufferDescVertex.ElementByteStride = sizeof(Vertex);
    RTBufferDescVertex.Size = sizeof(Vertex);
    contextData.device->CreateBuffer(RTBufferDescVertex, nullptr, &m_rtVertices);

    Diligent::BufferDesc RTBufferDescIndex;
    RTBufferDescIndex.Name = "Indices Buffer";
    RTBufferDescIndex.Usage = Diligent::USAGE_DEFAULT;
    RTBufferDescIndex.BindFlags = Diligent::BIND_UNORDERED_ACCESS;
    RTBufferDescIndex.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    RTBufferDescIndex.ElementByteStride = sizeof(unsigned int);
    RTBufferDescIndex.Size = sizeof(unsigned int);
    contextData.device->CreateBuffer(RTBufferDescIndex, nullptr, &m_rtIndices);

    /*Diligent::BufferDesc RTBufferDescVertexBVH;
    RTBufferDescVertexBVH.Name = "Vertices Bvh Buffer";
    RTBufferDescVertexBVH.Usage = Diligent::USAGE_DEFAULT;
    RTBufferDescVertexBVH.BindFlags = Diligent::BIND_UNORDERED_ACCESS;
    RTBufferDescVertexBVH.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    RTBufferDescVertexBVH.ElementByteStride = sizeof(BVH::Triangle);
    RTBufferDescVertexBVH.Size = sizeof(BVH::Triangle);
    contextData.device->CreateBuffer(RTBufferDescVertexBVH, nullptr, &m_rtBvhVertices);

    Diligent::BufferDesc RTBufferDescNodesBVH;
    RTBufferDescNodesBVH.Name = "Nodes Bvh Buffer";
    RTBufferDescNodesBVH.Usage = Diligent::USAGE_DEFAULT;
    RTBufferDescNodesBVH.BindFlags = Diligent::BIND_UNORDERED_ACCESS;
    RTBufferDescNodesBVH.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    RTBufferDescNodesBVH.ElementByteStride = sizeof(BVH::BVHNode);
    RTBufferDescNodesBVH.Size = sizeof(BVH::BVHNode);
    contextData.device->CreateBuffer(RTBufferDescNodesBVH, nullptr, &m_rtBvhNodes);*/

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "vertices")->Set(m_rtVertices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
    //m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "verticesBVH")->Set(m_rtBvhVertices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
    //m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "nodesBVH")->Set(m_rtBvhVertices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
    m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "indices")->Set(m_rtIndices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
    m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::MUTABLE_MODELS.c_str())->Set(MeshIndirect::getInstance().modelBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
    m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "SizeData")->Set(m_size->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));

    glm::ivec4 totalSize;

    totalSize.r = 0;

    contextData.immediateContext->UpdateBuffer(m_totalMeshes, 0, sizeof(glm::ivec4),
                                               glm::value_ptr(totalSize),
                                               Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    MeshIndirect::getInstance().addResizeHandler({"Software Raytracing handler" ,[&](Diligent::RefCntAutoPtr<Diligent::IBuffer> buffers, MeshIndirect::MaterialView& materials) {
        loadData();
    }});
}

void Prisma::PipelineSoftwareRT::render() {
    auto& contextData = PrismaFunc::getInstance().contextData();
    auto color = m_texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
    contextData.immediateContext->SetRenderTargets(1, &color, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    contextData.immediateContext->ClearRenderTarget(color, value_ptr(Define::CLEAR_COLOR),
                                                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    Diligent::DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = (m_width + 7) / 8;
    DispatAttribs.ThreadGroupCountY = (m_height + 7) / 8;
    DispatAttribs.ThreadGroupCountZ = 1;
    contextData.immediateContext->SetPipelineState(m_pso);
    contextData.immediateContext->CommitShaderResources(
        m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->DispatchCompute(DispatAttribs);
    m_blitRT->blit();
}

void Prisma::PipelineSoftwareRT::loadData() {
    if (Engine::getInstance().engineSettings().pipeline == EngineSettings::Pipeline::SOFTWARE_RAYTRACING) {
        auto& contextData = PrismaFunc::getInstance().contextData();

        auto& meshes = GlobalData::getInstance().currentGlobalScene()->meshes;
        if (!meshes.empty()) {
            m_rtVertices.Release();
            m_rtIndices.Release();
            //m_rtBvhVertices.Release();
            //m_rtBvhNodes.Release();
            m_size.Release();
            m_srb.Release();
            m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);

            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
            std::vector<Sizes> sizes;
            unsigned int currentVertex = 0;
            unsigned int currentIndex = 0;
            for (auto mesh : meshes) {
                for (auto v : mesh->verticesData().vertices) {
                    vertices.push_back({glm::vec4(v.position, 1), glm::vec4(v.texCoords, glm::vec2(1))});
                }
                for (auto v : mesh->verticesData().indices) {
                    indices.push_back(v);
                }
                Sizes size;
                size.vertexBase = currentVertex;
                size.indexBase = currentIndex;
                size.vertexSize = mesh->verticesData().vertices.size();
                size.indexSize = mesh->verticesData().indices.size();
                currentVertex = currentVertex + mesh->verticesData().vertices.size();
                currentIndex = currentIndex + mesh->verticesData().indices.size();

                sizes.push_back(size);
            }

            Diligent::BufferDesc RTBufferDescVertex;
            RTBufferDescVertex.Name = "Vertices Buffer";
            RTBufferDescVertex.Usage = Diligent::USAGE_DEFAULT;
            RTBufferDescVertex.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_UNORDERED_ACCESS;
            RTBufferDescVertex.Mode = Diligent::BUFFER_MODE_STRUCTURED;
            RTBufferDescVertex.ElementByteStride = sizeof(Vertex);
            RTBufferDescVertex.Size = sizeof(Vertex) * vertices.size();
            Diligent::BufferData vertexData;
            vertexData.DataSize = RTBufferDescVertex.Size;
            vertexData.pData = vertices.data();
            contextData.device->CreateBuffer(RTBufferDescVertex, &vertexData, &m_rtVertices);

            Diligent::BufferDesc RTBufferDescIndex;
            RTBufferDescIndex.Name = "Indices Buffer";
            RTBufferDescIndex.Usage = Diligent::USAGE_DEFAULT;
            RTBufferDescIndex.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_UNORDERED_ACCESS;
            RTBufferDescIndex.Mode = Diligent::BUFFER_MODE_STRUCTURED;
            RTBufferDescIndex.ElementByteStride = sizeof(unsigned int);
            RTBufferDescIndex.Size = sizeof(unsigned int) * indices.size();
            Diligent::BufferData indexData;
            indexData.DataSize = RTBufferDescIndex.Size;
            indexData.pData = indices.data();
            contextData.device->CreateBuffer(RTBufferDescIndex, &indexData, &m_rtIndices);

            Diligent::BufferDesc CBDesc;
            CBDesc.Name = "Sizes";
            CBDesc.Size = sizeof(Sizes);
            CBDesc.Usage = Diligent::USAGE_DEFAULT;
            CBDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS;
            CBDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
            CBDesc.ElementByteStride = sizeof(Sizes);
            CBDesc.Size = sizeof(Sizes) * sizes.size();
            Diligent::BufferData sizesData;
            sizesData.DataSize = CBDesc.Size;
            sizesData.pData = sizes.data();
            contextData.device->CreateBuffer(CBDesc, &sizesData, &m_size);

            glm::ivec4 totalSize;

            totalSize.r = meshes.size();

            contextData.immediateContext->UpdateBuffer(m_totalMeshes, 0, sizeof(glm::ivec4),
                                                       glm::value_ptr(totalSize),
                                                       Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            /*
            BVH bvh(vertices, indices, sizes);
            auto bvhNodes = bvh.getFlatNodes();
            auto bvhTriangles = bvh.getTriangles();
    
            Diligent::BufferDesc RTBufferDescVertexBVH;
            RTBufferDescVertexBVH.Name = "Vertices Bvh Buffer";
            RTBufferDescVertexBVH.Usage = Diligent::USAGE_DEFAULT;
            RTBufferDescVertexBVH.BindFlags = Diligent::BIND_UNORDERED_ACCESS;
            RTBufferDescVertexBVH.Mode = Diligent::BUFFER_MODE_STRUCTURED;
            RTBufferDescVertexBVH.ElementByteStride = sizeof(BVH::Triangle);
            RTBufferDescVertexBVH.Size = sizeof(BVH::Triangle) * bvhTriangles.size();
            Diligent::BufferData bvhvData;
            bvhvData.DataSize = RTBufferDescVertexBVH.Size;
            bvhvData.pData = bvhTriangles.data();
    
            contextData.device->CreateBuffer(RTBufferDescVertexBVH, &bvhvData, &m_rtBvhVertices);
    
            Diligent::BufferDesc RTBufferDescNodesBVH;
            RTBufferDescNodesBVH.Name = "Nodes Bvh Buffer";
            RTBufferDescNodesBVH.Usage = Diligent::USAGE_DEFAULT;
            RTBufferDescNodesBVH.BindFlags = Diligent::BIND_UNORDERED_ACCESS;
            RTBufferDescNodesBVH.Mode = Diligent::BUFFER_MODE_STRUCTURED;
            RTBufferDescNodesBVH.ElementByteStride = sizeof(BVH::BVHNode);
            RTBufferDescNodesBVH.Size = sizeof(BVH::BVHNode) * bvhNodes.size();
            Diligent::BufferData bvhnData;
            bvhnData.DataSize = RTBufferDescNodesBVH.Size;
            bvhnData.pData = bvhNodes.data();
    
            contextData.device->CreateBuffer(RTBufferDescNodesBVH, &bvhnData, &m_rtBvhNodes);*/
            auto materials = MeshIndirect::getInstance().textureViews();
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "vertices")->Set(m_rtVertices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "indices")->Set(m_rtIndices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
            //m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "verticesBVH")->Set(m_rtBvhVertices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
            //m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "nodesBVH")->Set(m_rtBvhVertices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::MUTABLE_MODELS.c_str())->Set(MeshIndirect::getInstance().modelBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "SizeData")->Set(m_size->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str())->SetArray(materials.diffuse.data(), 0, materials.diffuse.size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
        }
    }
}