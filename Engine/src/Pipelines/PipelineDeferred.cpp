#include <Graphics/GraphicsTools/interface/MapHelper.hpp>
#include <iostream>
#include <memory>
#include <random>
#include <string>

#include "Common/interface/RefCntAutoPtr.hpp"
#include "GlobalData/GlobalData.h"
#include "GlobalData/GlobalShaderNames.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"
#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#include "Graphics/GraphicsTools/interface/GraphicsUtilities.h"
#include "Handlers/ComponentsHandler.h"
#include "Handlers/LightHandler.h"
#include "Helpers/ClusterCalculation.h"
#include "Helpers/PrismaRender.h"
#include "Helpers/SettingsLoader.h"
#include "Pipelines/PipelineDIffuseIrradiance.h"
#include "Pipelines/PipelineDeferred.h"
#include "Pipelines/PipelineHandler.h"
#include "Pipelines/PipelineLUT.h"
#include "Pipelines/PipelinePrefilter.h"
#include "Pipelines/PipelineSkybox.h"
#include "Postprocess/Postprocess.h"
#include "SceneData/MeshIndirect.h"
#include "SceneObjects/Mesh.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "engine.h"

using namespace Diligent;

Prisma::PipelineDeferred::PipelineDeferred(Diligent::RefCntAutoPtr<Diligent::ITexture> albedo, Diligent::RefCntAutoPtr<Diligent::ITexture> normal, Diligent::RefCntAutoPtr<Diligent::ITexture> position)
    : m_albedoTexture{albedo}, m_normalTexture{normal}, m_positionTexture{position} {
    create();
}

void Prisma::PipelineDeferred::render() {
    auto& contextData = PrismaFunc::getInstance().contextData();

            
    if (Prisma::Engine::getInstance().engineSettings().ssao) {
        m_ssao->render();
    } else {
        auto color = m_ssao->ssaoTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

        contextData.immediateContext->SetRenderTargets(1, &color, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        contextData.immediateContext->ClearRenderTarget(color, glm::value_ptr(glm::vec4(1)), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    auto color = Prisma::PipelineHandler::getInstance().textureData().pColorRTV->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

    contextData.immediateContext->SetRenderTargets(1, &color, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->ClearRenderTarget(color, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->SetPipelineState(m_pso);

    auto quadBuffer = PrismaRender::getInstance().quadBuffer();

    // Bind vertex and index buffers
    constexpr Diligent::Uint64 offset = 0;
    Diligent::IBuffer* pBuffs[] = {quadBuffer.vBuffer};
    contextData.immediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    contextData.immediateContext->SetIndexBuffer(quadBuffer.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    // Set texture SRV in the SRB
    contextData.immediateContext->CommitShaderResources(m_srbOpaque, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Diligent::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
    DrawAttrs.IndexType = Diligent::VT_UINT32;  // Index type
    DrawAttrs.NumIndices = quadBuffer.iBufferSize;
    // Verify the state of vertex and index buffers
    DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    contextData.immediateContext->DrawIndexed(DrawAttrs);
}

Prisma::PipelineDeferred::~PipelineDeferred() {}

void Prisma::PipelineDeferred::create() {
    auto& contextData = PrismaFunc::getInstance().contextData();

    m_ssao = std::make_unique<PipelineSSAO>(m_normalTexture, m_positionTexture);

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Deferred Render";

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
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    contextData.engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Deferred VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/DeferredPipeline/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Deferred PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/DeferredPipeline/fragment.glsl";
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
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    std::string samplerClampName = "textureClamp_sampler";

    PipelineResourceDesc Resources[] = {

        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_LIGHT_SIZES.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_OMNI_DATA.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_CLUSTERS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_CLUSTERS_DATA.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_OMNI_DATA_SHADOW.c_str(), Define::MAX_OMNI_SHADOW, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL, "albedo", 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, "normal", 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, "position", 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, "ssao", 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIR_SHADOW.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL, samplerClampName.c_str(), 1, SHADER_RESOURCE_TYPE_SAMPLER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_LUT.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_PREFILTER.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_IRRADIANCE.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},

    };

    PipelineResourceSignatureDesc ResourceSignDesc;
    ResourceSignDesc.NumResources = _countof(Resources);
    ResourceSignDesc.Resources = Resources;

    // clang-format off
    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    SamplerDesc SamLinearClampDesc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
    };

    RefCntAutoPtr<ISampler> samplerClamp;

    contextData.device->CreatePipelineResourceSignature(ResourceSignDesc, &m_pResourceSignature);

    IPipelineResourceSignature* ppSignatures[]{ m_pResourceSignature };

    PSOCreateInfo.ppResourceSignatures = ppSignatures;
    PSOCreateInfo.ResourceSignaturesCount = _countof(ppSignatures);


    contextData.device->CreatePipelineState(PSOCreateInfo, &m_pso);
    contextData.device->CreateSampler(SamLinearClampDesc, &samplerClamp);



    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str())->Set(CSMHandler::getInstance().shadowBuffer());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_OMNI_DATA.c_str())->Set(LightHandler::getInstance().omniLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA.c_str())->Set(LightHandler::getInstance().dirLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_CLUSTERS.c_str())->Set(LightHandler::getInstance().clusters().clusters->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_CLUSTERS_DATA.c_str())->Set(LightHandler::getInstance().clusters().clustersData);

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_LIGHT_SIZES.c_str())->Set(LightHandler::getInstance().lightSizes());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_LUT.c_str())->Set(PipelineLUT::getInstance().lutTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, "albedo")->Set(m_albedoTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, "normal")->Set(m_normalTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, "position")->Set(m_positionTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, "ssao")->Set(m_ssao->ssaoTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

    IDeviceObject* samplerDeviceClamp = samplerClamp;

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, samplerClampName.c_str())->Set(samplerDeviceClamp);

    // Create a shader resource binding object and bind all static resources in it
    m_pResourceSignature->CreateShaderResourceBinding(&m_srbOpaque, true);

    m_updateData = [&](RefCntAutoPtr<IShaderResourceBinding>& srb,RefCntAutoPtr<IBuffer>& indexBuffer)
        {
            auto buffers = MeshIndirect::getInstance().modelBuffer();
            auto materials = MeshIndirect::getInstance().textureViews();
            auto status = MeshIndirect::getInstance().statusBuffer();
            srb.Release();
            m_pResourceSignature->CreateShaderResourceBinding(&srb, true);
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_OMNI_DATA_SHADOW.c_str())->SetArray(LightHandler::getInstance().omniData().data(), 0, LightHandler::getInstance().omniData().size(), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIR_SHADOW.c_str())->Set(LightHandler::getInstance().dirShadowData());
            if (PipelineSkybox::getInstance().isInit()) {
                srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_PREFILTER.c_str())->Set(PipelinePrefilter::getInstance().prefilterTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
                srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_IRRADIANCE.c_str())->Set(PipelineDiffuseIrradiance::getInstance().irradianceTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
            }
        };
    m_updateData(m_srbOpaque,MeshIndirect::getInstance().indexBufferOpaque());
    MeshIndirect::getInstance().addResizeHandler({"DeferredMesh handler" ,[&](RefCntAutoPtr<IBuffer> buffers, MeshIndirect::MaterialView& materials)
        {
            m_updateData(m_srbOpaque,MeshIndirect::getInstance().indexBufferOpaque());
        }});
    PipelineSkybox::getInstance().addUpdate({"DeferredMesh",[&]()
        {
            m_updateData(m_srbOpaque,MeshIndirect::getInstance().indexBufferOpaque());
        }});
    LightHandler::getInstance().addLightHandler({"DeferredMesh",[&]()
        {
            m_updateData(m_srbOpaque,MeshIndirect::getInstance().indexBufferOpaque());
        }});
}