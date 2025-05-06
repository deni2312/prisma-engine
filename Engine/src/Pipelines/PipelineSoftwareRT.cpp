#include "Pipelines/PipelineSoftwareRT.h"

#include "GlobalData/PrismaFunc.h"
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

    Diligent::ShaderResourceVariableDesc Vars[] = {
        {Diligent::SHADER_TYPE_COMPUTE, "screenTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {Diligent::SHADER_TYPE_COMPUTE, "vertices", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_COMPUTE, "indices", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}};
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    Diligent::SamplerDesc SamLinearClampDesc{Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};
    Diligent::ImmutableSamplerDesc ImtblSamplers[] = {{Diligent::SHADER_TYPE_PIXEL, "screenTexture", SamLinearClampDesc}};
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
    PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

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
    PSODesc.Name = "Software RT";
    PSOCreateInfo.pCS = pResetParticleListsCS;
    contextData.device->CreateComputePipelineState(PSOCreateInfo, &m_pso);
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "screenTexture")->Set(m_texture->GetDefaultView(Diligent::TEXTURE_VIEW_UNORDERED_ACCESS));

    m_pso->CreateShaderResourceBinding(&m_srb, true);
    m_blit = std::make_unique<Blit>(m_texture);
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

    MeshIndirect::getInstance().addResizeHandler([&](Diligent::RefCntAutoPtr<Diligent::IBuffer> buffers, MeshIndirect::MaterialView& materials) {
        auto& meshes = GlobalData::getInstance().currentGlobalScene()->meshes;
        if (!meshes.empty()) {
            m_rtVertices.Release();
            m_rtIndices.Release();
            m_srb.Release();
            m_pso->CreateShaderResourceBinding(&m_srb, true);

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
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "vertices")->Set(m_rtVertices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "indices")->Set(m_rtIndices->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
        }
    });
}

void Prisma::PipelineSoftwareRT::render() {
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = (m_width + 7) / 8;
    DispatAttribs.ThreadGroupCountY = (m_height + 7) / 8;
    DispatAttribs.ThreadGroupCountZ = 1;
    contextData.immediateContext->SetPipelineState(m_pso);
    contextData.immediateContext->CommitShaderResources(
        m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->DispatchCompute(DispatAttribs);
    m_blit->render(PipelineHandler::getInstance().textureData().pColorRTV);
}