#include "Pipelines/PipelineSoftwareRT.h"

#include "GlobalData/GlobalShaderNames.h"
#include "GlobalData/PrismaFunc.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Pipelines/PipelineHandler.h"

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
    Diligent::PipelineResourceDesc Resources[] =
    {
        {Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {Diligent::SHADER_TYPE_COMPUTE, ShaderNames::MUTABLE_MODELS.c_str(), 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_COMPUTE, "SizeData", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {Diligent::SHADER_TYPE_COMPUTE, "vertices", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_COMPUTE, "indices", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_COMPUTE, "screenTexture", 1, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
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
    CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    contextData.device->CreateBuffer(CBDesc, nullptr, &m_size);

    m_pResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "screenTexture")->Set(m_texture->GetDefaultView(Diligent::TEXTURE_VIEW_UNORDERED_ACCESS));
    m_pResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());
    m_pResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "SizeData")->Set(m_size);

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
    RTBufferDescIndex.ElementByteStride = sizeof(glm::ivec4);
    RTBufferDescIndex.Size = sizeof(glm::ivec4);
    contextData.device->CreateBuffer(RTBufferDescIndex, nullptr, &m_rtIndices);

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "vertices")->Set(m_rtVertices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
    m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "indices")->Set(m_rtIndices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
    m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::MUTABLE_MODELS.c_str())->Set(MeshIndirect::getInstance().modelBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));

    MeshIndirect::getInstance().addResizeHandler([&](Diligent::RefCntAutoPtr<Diligent::IBuffer> buffers, MeshIndirect::MaterialView& materials) {
        auto& meshes = GlobalData::getInstance().currentGlobalScene()->meshes;
        if (!meshes.empty()) {
            m_rtVertices.Release();
            m_rtIndices.Release();
            m_srb.Release();
            m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);

            auto mesh = meshes[0];
            std::vector<Vertex> vertices;
            std::vector<glm::ivec4> indices;
            for (auto v : mesh->verticesData().vertices) {
                vertices.push_back({glm::vec4(v.position, 1)});
            }
            for (auto v : mesh->verticesData().indices) {
                indices.push_back({glm::ivec4(v)});
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
            RTBufferDescIndex.ElementByteStride = sizeof(glm::ivec4);
            RTBufferDescIndex.Size = sizeof(glm::ivec4) * indices.size();
            Diligent::BufferData indexData;
            indexData.DataSize = RTBufferDescIndex.Size;
            indexData.pData = indices.data();
            contextData.device->CreateBuffer(RTBufferDescIndex, &indexData, &m_rtIndices);
            Sizes sizes;
            sizes.vertexSize = vertices.size();
            sizes.indexSize = indices.size();

            contextData.immediateContext->UpdateBuffer(m_size, 0, sizeof(Sizes),
                                                       &sizes,
                                                       Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "vertices")->Set(m_rtVertices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "indices")->Set(m_rtIndices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::MUTABLE_MODELS.c_str())->Set(buffers->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
        }
    });
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