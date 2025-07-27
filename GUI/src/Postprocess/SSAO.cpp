#include "Postprocess/SSAO.h"


#include "GlobalData/PrismaFunc.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Helpers/PrismaRender.h"
#include "Pipelines/PipelineHandler.h"
#include <GlobalData/GlobalShaderNames.h>
#include "Helpers/SettingsLoader.h"


void Prisma::GUI::SSAO::render() {
    if (m_apply) {
        auto& contextData = PrismaFunc::getInstance().contextData();

        auto color = m_texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

        contextData.immediateContext->SetRenderTargets(1, &color, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        contextData.immediateContext->SetPipelineState(m_pso);

        auto quadBuffer = PrismaRender::getInstance().quadBuffer();

        // Bind vertex and index buffers
        constexpr Diligent::Uint64 offset = 0;
        Diligent::IBuffer* pBuffs[] = {quadBuffer.vBuffer};
        contextData.immediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        contextData.immediateContext->SetIndexBuffer(quadBuffer.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        // Set texture SRV in the SRB
        contextData.immediateContext->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Diligent::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
        DrawAttrs.IndexType = Diligent::VT_UINT32;  // Index type
        DrawAttrs.NumIndices = quadBuffer.iBufferSize;
        // Verify the state of vertex and index buffers
        DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        contextData.immediateContext->DrawIndexed(DrawAttrs);
        m_blit->render(PipelineHandler::getInstance().textureData().pColorRTV);
    }
}

Prisma::GUI::SSAO::SSAO() {
    auto& contextData = PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "SSAO Render";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = PipelineHandler::getInstance().textureFormat();

    // Set depth buffer format which is the format of the swap chain's back buffer
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;
    // clang-format on

    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;
    // In this tutorial, we will load shaders from file. To be able to do that,
    // we need to create a shader source stream factory
    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    PrismaFunc::getInstance().contextData().engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);

    auto settings = Prisma::SettingsLoader::getInstance().getSettings();

    std::string widthStr = std::to_string(static_cast<int>(settings.width));
    std::string heightStr = std::to_string(static_cast<int>(settings.height));

    Diligent::ShaderMacro Macros[] = {{"WIDTH", widthStr.c_str()}, {"HEIGHT", heightStr.c_str()}};

    ShaderCI.Macros = {Macros, _countof(Macros)};

    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "SSAO VS";
        ShaderCI.FilePath = "../../../GUI/Shaders/SSAO/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "SSAO PS";
        ShaderCI.FilePath = "../../../GUI/Shaders/SSAO/fragment.glsl";
        contextData.device->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
	Diligent::LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, Diligent::False},
        // Attribute 1 - texture coordinates
        Diligent::LayoutElement{1, 0, 2, Diligent::VT_FLOAT32, Diligent::False}
    };
    // clang-format on
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    Diligent::ShaderResourceVariableDesc Vars[] = {{Diligent::SHADER_TYPE_PIXEL, "screenTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    Diligent::SamplerDesc SamLinearClampDesc{Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};
    Diligent::SamplerDesc SamLinearClampNear{Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP};

    Diligent::ImmutableSamplerDesc ImtblSamplers[] = {{Diligent::SHADER_TYPE_PIXEL, "screenTexture", SamLinearClampDesc}, {Diligent::SHADER_TYPE_PIXEL, "positionTexture", SamLinearClampNear}};
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
    PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
    contextData.device->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    RTColorDesc.Width = contextData.swapChain->GetDesc().Width;
    RTColorDesc.Height = contextData.swapChain->GetDesc().Height;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = PipelineHandler::getInstance().textureFormat();
    // The render target can be bound as a shader resource and as a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.ClearValue.Color[0] = 0.350f;
    RTColorDesc.ClearValue.Color[1] = 0.350f;
    RTColorDesc.ClearValue.Color[2] = 0.350f;
    RTColorDesc.ClearValue.Color[3] = 1.f;
    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_texture);

    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);  // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<glm::vec4> ssaoKernel;
    const unsigned int size = 64;
    for (unsigned int i = 0; i < size; ++i) {
        glm::vec4 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator),0);
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / size;

        // scale samples s.t. they're more aligned to center of kernel
        scale = ourLerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    Diligent::BufferDesc OmniDesc;
    OmniDesc.Name = "SSAO Buffer";
    OmniDesc.Usage = Diligent::USAGE_DEFAULT;
    OmniDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    OmniDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    OmniDesc.ElementByteStride = sizeof(glm::vec4);
    OmniDesc.Size = size * sizeof(glm::vec4);

    Diligent::BufferData bufferData;

    bufferData.DataSize = OmniDesc.Size;
    bufferData.pData = ssaoKernel.data();

    contextData.device->CreateBuffer(OmniDesc, &bufferData, &m_buffer);
    // generate noise texture
    // ----------------------
    std::vector<glm::vec4> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec4 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f,1);  // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }

    // 3. Create texture
    Diligent::TextureDesc TexDesc;
    TexDesc.Name = "SSAO Noise Texture";
    TexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    TexDesc.Width = 4;
    TexDesc.Height = 4;
    TexDesc.Format = Diligent::TEX_FORMAT_RGBA32_FLOAT;
    TexDesc.Usage = Diligent::USAGE_DEFAULT;
    TexDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    TexDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;

    // Fill Subresource data
    Diligent::TextureSubResData SubresData;
    SubresData.pData = ssaoNoise.data();
    SubresData.Stride = 4 * sizeof(float) * TexDesc.Width;  // row pitch in bytes

    Diligent::TextureData InitData;
    InitData.pSubResources = &SubresData;
    InitData.NumSubresources = 1;

    contextData.device->CreateTexture(TexDesc, &InitData, &m_noiseTexture);


    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "noiseTexture")->Set(m_noiseTexture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "normalTexture")->Set(PipelineHandler::getInstance().deferredForward()->normalTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "positionTexture")->Set(PipelineHandler::getInstance().deferredForward()->positionTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")->Set(m_buffer);
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());

    m_pso->CreateShaderResourceBinding(&m_srb, true);

    m_blit = std::make_unique<Blit>(m_texture);
    GlobalData::getInstance().addGlobalTexture({m_texture, "SSAO Texture"});
    GlobalData::getInstance().addGlobalTexture({m_noiseTexture, "SSAO noise"});


}


void Prisma::GUI::SSAO::apply(bool apply) { m_apply = apply;}

bool Prisma::GUI::SSAO::apply() { return m_apply; }

float Prisma::GUI::SSAO::ourLerp(float a, float b, float f) { return a + f * (b - a); }
