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
#include "Pipelines/PipelineDeferredForward.h"
#include "Pipelines/PipelineHandler.h"
#include "Pipelines/PipelineLUT.h"
#include "Pipelines/PipelinePrefilter.h"
#include "Pipelines/PipelineSkybox.h"
#include "Postprocess/Postprocess.h"
#include "SceneData/MeshIndirect.h"
#include "SceneObjects/Mesh.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

using namespace Diligent;

Prisma::PipelineDeferredForward::PipelineDeferredForward(const unsigned int& width, const unsigned int& height) : m_width{width}, m_height{height} {
    create();
    createAnimation();
}

void Prisma::PipelineDeferredForward::render() {
    auto& contextData = PrismaFunc::getInstance().contextData();

    auto pDSV = PipelineHandler::getInstance().textureData().pDepthDSV->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);


    ITextureView* textures[] = {
        m_positionTexture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET),
        m_normalTexture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET),
        m_albedoTexture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET),
    };

    // Clear the back buffer
    contextData.immediateContext->SetRenderTargets(3, textures, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // contextData.immediateContext->ClearRenderTarget(pRTV, value_ptr(Define::CLEAR_COLOR),RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->ClearRenderTarget(textures[0], value_ptr(Define::CLEAR_COLOR), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->ClearRenderTarget(textures[1], value_ptr(Define::CLEAR_COLOR), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->ClearRenderTarget(textures[2], value_ptr(Define::CLEAR_COLOR), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    contextData.immediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Prisma::ComponentsHandler::getInstance().updatePreRender(PipelineHandler::getInstance().textureData().pColorRTV, PipelineHandler::getInstance().textureData().pDepthDSV);

    // Set the pipeline state
    contextData.immediateContext->SetPipelineState(m_pso);
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    auto& meshes = GlobalData::getInstance().currentGlobalScene()->meshes;
    auto& meshesAnimation = GlobalData::getInstance().currentGlobalScene()->animateMeshes;
    if (!meshes.empty() && PipelineSkybox::getInstance().isInit()) {
        MeshIndirect::getInstance().setupBuffers();
        // Set texture SRV in the SRB
        contextData.immediateContext->CommitShaderResources(m_srbOpaque, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        MeshIndirect::getInstance().renderMeshesOpaque();
    }
    
    contextData.immediateContext->SetPipelineState(m_psoAnimation);
    if (!meshesAnimation.empty() && PipelineSkybox::getInstance().isInit()) {
        MeshIndirect::getInstance().setupBuffersAnimation();
        // Set texture SRV in the SRB
        contextData.immediateContext->CommitShaderResources(m_srbAnimation, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        MeshIndirect::getInstance().renderAnimateMeshes();
    }

    if ((!meshes.empty() || !meshesAnimation.empty()) && PipelineSkybox::getInstance().isInit()) {
        m_deferredPipeline->render();
    }

    auto pRTV = PipelineHandler::getInstance().textureData().pColorRTV->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);

    // Clear the back buffer
    contextData.immediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    //Prisma::ComponentsHandler::getInstance().updatePostRender(PipelineHandler::getInstance().textureData().pColorRTV, PipelineHandler::getInstance().textureData().pDepthDSV);

    PipelineSkybox::getInstance().render();

    //m_forwardTransparent->render();

    auto& sprites = GlobalData::getInstance().currentGlobalScene()->sprites;

    for (auto& sprite : sprites) {
        sprite->render();
    }
    Physics::getInstance().drawDebug();
    //renderComposite();
    
    // m_blit->render(PipelineHandler::getInstance().textureData().pColorRTV);
    PrismaFunc::getInstance().bindMainRenderTarget();
}

Prisma::PipelineDeferredForward::~PipelineDeferredForward() {}

void Prisma::PipelineDeferredForward::create() {
    auto& contextData = PrismaFunc::getInstance().contextData();

    GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Deferred Forward Pipeline";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 3;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PipelineHandler::getInstance().textureFormat();
    PSOCreateInfo.GraphicsPipeline.RTVFormats[1] = Prisma::PipelineHandler::getInstance().textureFormat();
    PSOCreateInfo.GraphicsPipeline.RTVFormats[2] = Prisma::PipelineHandler::getInstance().textureFormat();
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
    // clang-format on

    ShaderCreateInfo ShaderCI;

    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_GLSL;
    ShaderCI.CompileFlags |= SHADER_COMPILE_FLAG_ENABLE_UNBOUNDED_ARRAYS;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;

    // Pack matrices in row-major order

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
        ShaderCI.Desc.Name = "Deferred Forward VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/DeferredForwardPipeline/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Deferred Forward PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/DeferredForwardPipeline/fragment.glsl";
        contextData.device->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
    LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        LayoutElement{0, 0, 3, VT_FLOAT32, False},
        // Attribute 1 - texture coordinates
        LayoutElement{1, 0, 3, VT_FLOAT32, False},

        LayoutElement{2, 0, 2, VT_FLOAT32, False},

        LayoutElement{3, 0, 3, VT_FLOAT32, False},

        LayoutElement{4, 0, 3, VT_FLOAT32, False}
    };
    // clang-format on
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    std::string samplerRepeatName = "textureRepeat_sampler";

    PipelineResourceDesc Resources[] = {
        {SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_INDEX_OPAQUE.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},

        {SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str(), Define::MAX_MESHES, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str(), Define::MAX_MESHES, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str(), Define::MAX_MESHES, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},

        {SHADER_TYPE_PIXEL, samplerRepeatName.c_str(), 1, SHADER_RESOURCE_TYPE_SAMPLER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_STATUS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
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

    SamplerDesc SamLinearRepeatDesc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP
    };

    RefCntAutoPtr<ISampler> samplerRepeat;

    contextData.device->CreatePipelineResourceSignature(ResourceSignDesc, &m_pResourceSignature);

    IPipelineResourceSignature* ppSignatures[]{ m_pResourceSignature };

    PSOCreateInfo.ppResourceSignatures = ppSignatures;
    PSOCreateInfo.ResourceSignaturesCount = _countof(ppSignatures);


    contextData.device->CreatePipelineState(PSOCreateInfo, &m_pso);
    contextData.device->CreateSampler(SamLinearRepeatDesc, &samplerRepeat);


    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());
    IDeviceObject* samplerDeviceRepeat = samplerRepeat;

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, samplerRepeatName.c_str())->Set(samplerDeviceRepeat);

    // Create a shader resource binding object and bind all static resources in it
    m_pResourceSignature->CreateShaderResourceBinding(&m_srbOpaque, true);

    m_updateData = [&](RefCntAutoPtr<IShaderResourceBinding>& srb,RefCntAutoPtr<IBuffer>& indexBuffer)
        {
            auto buffers = MeshIndirect::getInstance().modelBuffer();
            auto materials = MeshIndirect::getInstance().textureViews();
            auto status = MeshIndirect::getInstance().statusBuffer();
            srb.Release();
            m_pResourceSignature->CreateShaderResourceBinding(&srb, true);
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str())->SetArray(materials.diffuse.data(), 0, materials.diffuse.size(), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str())->SetArray(materials.normal.data(), 0, materials.normal.size(), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str())->SetArray(materials.rm.data(), 0, materials.rm.size(), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            srb->GetVariableByName(SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->Set(buffers->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            srb->GetVariableByName(SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_INDEX_OPAQUE.c_str())->Set(indexBuffer->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            
            if (status) {
                srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_STATUS.c_str())->Set(status->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            }
        };
    m_updateData(m_srbOpaque,MeshIndirect::getInstance().indexBufferOpaque());
    //CreateMSAARenderTarget();
    MeshIndirect::getInstance().addResizeHandler({"DeferredForwardMesh handler" ,[&](RefCntAutoPtr<IBuffer> buffers, MeshIndirect::MaterialView& materials)
        {
            m_updateData(m_srbOpaque,MeshIndirect::getInstance().indexBufferOpaque());
        }});
    PipelineSkybox::getInstance().addUpdate({"DeferredForwardMesh",[&]()
        {
            m_updateData(m_srbOpaque,MeshIndirect::getInstance().indexBufferOpaque());
        }});
    LightHandler::getInstance().addLightHandler({"DeferredForwardMesh",[&]()
        {
            m_updateData(m_srbOpaque,MeshIndirect::getInstance().indexBufferOpaque());
        }});

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "ALbedo Render Target";
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
    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_albedoTexture);
    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_normalTexture);
    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_positionTexture);

    GlobalData::getInstance().addGlobalTexture({m_albedoTexture, "Deferred Albedo Texture"});
    GlobalData::getInstance().addGlobalTexture({m_normalTexture, "Deferred Normal Texture"});
    GlobalData::getInstance().addGlobalTexture({m_positionTexture, "Deferred Position Texture"});

    m_deferredPipeline=std::make_unique<PipelineDeferred>(m_albedoTexture,m_normalTexture,m_positionTexture);
}

void Prisma::PipelineDeferredForward::createAnimation()
{
    auto& contextData = PrismaFunc::getInstance().contextData();

    GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Deferred Forward Pipeline";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 3;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PipelineHandler::getInstance().textureFormat();
    PSOCreateInfo.GraphicsPipeline.RTVFormats[1] = Prisma::PipelineHandler::getInstance().textureFormat();
    PSOCreateInfo.GraphicsPipeline.RTVFormats[2] = Prisma::PipelineHandler::getInstance().textureFormat();
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
    // clang-format on

    ShaderCreateInfo ShaderCI;

    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_GLSL;
    ShaderCI.CompileFlags |= SHADER_COMPILE_FLAG_ENABLE_UNBOUNDED_ARRAYS;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;
    ShaderMacro Macros[] = {{"ANIMATION", "1"}};
    ShaderCI.Macros = {Macros, _countof(Macros)};
    // Pack matrices in row-major order

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
        ShaderCI.Desc.Name = "Deferred Forward VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/DeferredForwardPipeline/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Deferred Forward PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/DeferredForwardPipeline/fragment.glsl";
        contextData.device->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
    LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        LayoutElement{0, 0, 3, VT_FLOAT32, False},
        // Attribute 1 - texture coordinates
        LayoutElement{1, 0, 3, VT_FLOAT32, False},

        LayoutElement{2, 0, 2, VT_FLOAT32, False},

        LayoutElement{3, 0, 3, VT_FLOAT32, False},

        LayoutElement{4, 0, 3, VT_FLOAT32, False},

        LayoutElement{5, 0, 4, VT_INT32, False},

        LayoutElement{6, 0, 4, VT_FLOAT32, False},

    };
    // clang-format on
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    std::string samplerRepeatName = "textureRepeat_sampler";

    PipelineResourceDesc Resources[] = {
        {SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_ANIMATION.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_STATUS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str(), Define::MAX_MESHES, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str(), Define::MAX_MESHES, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str(), Define::MAX_MESHES, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},

        {SHADER_TYPE_PIXEL, samplerRepeatName.c_str(), 1, SHADER_RESOURCE_TYPE_SAMPLER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

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

    SamplerDesc SamLinearRepeatDesc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP
    };

    RefCntAutoPtr<ISampler> samplerRepeat;

    contextData.device->CreatePipelineResourceSignature(ResourceSignDesc, &m_pResourceSignatureAnimation);

    IPipelineResourceSignature* ppSignatures[]{ m_pResourceSignatureAnimation };

    PSOCreateInfo.ppResourceSignatures = ppSignatures;
    PSOCreateInfo.ResourceSignaturesCount = _countof(ppSignatures);


    contextData.device->CreatePipelineState(PSOCreateInfo, &m_psoAnimation);
    contextData.device->CreateSampler(SamLinearRepeatDesc, &samplerRepeat);


    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_ANIMATION.c_str())->Set(AnimationHandler::getInstance().animation()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));


    IDeviceObject* samplerDeviceRepeat = samplerRepeat;

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, samplerRepeatName.c_str())->Set(samplerDeviceRepeat);

    // Create a shader resource binding object and bind all static resources in it
    m_pResourceSignatureAnimation->CreateShaderResourceBinding(&m_srbAnimation, true);

    m_updateDataAnimation = [&]()
        {
            auto buffers = MeshIndirect::getInstance().modelBufferAnimation();
            auto materials = MeshIndirect::getInstance().textureViewsAnimation();
            auto status = MeshIndirect::getInstance().statusBufferAnimation();
            m_srbAnimation.Release();
            m_pResourceSignatureAnimation->CreateShaderResourceBinding(&m_srbAnimation, true);
            m_srbAnimation->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str())->SetArray(materials.diffuse.data(), 0, materials.diffuse.size(), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srbAnimation->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str())->SetArray(materials.normal.data(), 0, materials.normal.size(), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srbAnimation->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str())->SetArray(materials.rm.data(), 0, materials.rm.size(), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srbAnimation->GetVariableByName(SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->Set(buffers->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            if (status) {
                m_srbAnimation->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_STATUS.c_str())->Set(status->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            }
        };

    //CreateMSAARenderTarget();
    MeshIndirect::getInstance().addResizeHandler({"DeferredForwardAnimation handler" ,[&](RefCntAutoPtr<IBuffer> buffers, MeshIndirect::MaterialView& materials)
        {
            m_updateDataAnimation();
        }});
    PipelineSkybox::getInstance().addUpdate({"DeferredForwardAnimation",[&]()
        {
            m_updateDataAnimation();
        }});
    LightHandler::getInstance().addLightHandler({"DeferredForwardAnimation",[&]()
        {
            m_updateDataAnimation();
        }});



}
