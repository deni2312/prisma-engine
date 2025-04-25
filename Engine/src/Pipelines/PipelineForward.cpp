#include "Pipelines/PipelineForward.h"

#include "GlobalData/GlobalData.h"
#include "Pipelines/PipelineForward.h"
#include "Helpers/PrismaRender.h"
#include "SceneData/MeshIndirect.h"
#include "SceneObjects/Mesh.h"
#include "Pipelines/PipelineSkybox.h"
#include "Pipelines/PipelineDIffuseIrradiance.h"
#include "Pipelines/PipelinePrefilter.h"
#include "Pipelines/PipelineLUT.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <memory>
#include <iostream>
#include "Helpers/SettingsLoader.h"
#include "Helpers/ClusterCalculation.h"
#include <random>
#include "Postprocess/Postprocess.h"
#include "Handlers/ComponentsHandler.h"

#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

#include "Common/interface/RefCntAutoPtr.hpp"

#include "Graphics/GraphicsTools/interface/GraphicsUtilities.h"
#include <Graphics/GraphicsTools/interface/MapHelper.hpp>

#include "Handlers/LightHandler.h"
#include "Pipelines/PipelineHandler.h"
#include "GlobalData/GlobalShaderNames.h"
#include <string>

using namespace Diligent;

Prisma::PipelineForward::PipelineForward(const unsigned int& width, const unsigned int& height) : m_width{
	                                                                                                             width
                                                                                                             }, m_height{height}
{
    create();
    createAnimation();
}

void Prisma::PipelineForward::render(){
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    auto pRTV = Prisma::PipelineHandler::getInstance().textureData().pColorRTV->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
    auto pDSV = Prisma::PipelineHandler::getInstance().textureData().pDepthDSV->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
    // Clear the back buffer
    contextData.m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    contextData.m_pImmediateContext->ClearRenderTarget(pRTV, glm::value_ptr(Define::CLEAR_COLOR), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set the pipeline state
    contextData.m_pImmediateContext->SetPipelineState(m_pso);
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    auto& meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
    auto& meshesAnimation = Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes;
    if (!meshes.empty() && Prisma::PipelineSkybox::getInstance().isInit())
    {
        Prisma::MeshIndirect::getInstance().setupBuffers();
        // Set texture SRV in the SRB
        contextData.m_pImmediateContext->CommitShaderResources(m_srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        Prisma::MeshIndirect::getInstance().renderMeshes();
    }

    contextData.m_pImmediateContext->SetPipelineState(m_psoAnimation);
    if (!meshesAnimation.empty() && Prisma::PipelineSkybox::getInstance().isInit())
    {
        Prisma::MeshIndirect::getInstance().setupBuffersAnimation();
        // Set texture SRV in the SRB
        contextData.m_pImmediateContext->CommitShaderResources(m_srbAnimation, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        Prisma::MeshIndirect::getInstance().renderAnimateMeshes();
    }

    auto& sprites = Prisma::GlobalData::getInstance().currentGlobalScene()->sprites;

	for (auto& sprite : sprites)
	{
		sprite->render();
	}

    Prisma::PipelineSkybox::getInstance().render();

    Prisma::PrismaFunc::getInstance().bindMainRenderTarget();
}

Prisma::PipelineForward::~PipelineForward()
{
}

void Prisma::PipelineForward::create()
{
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Forward Pipeline";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PrismaFunc::getInstance().renderFormat().RenderFormat;
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = Prisma::PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
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
    contextData.m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Forward VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/ForwardPipeline/vertex.glsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pVS);
    }


    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Forward PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/ForwardPipeline/fragment.glsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pPS);
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


    std::string samplerClampName = "textureClamp_sampler";
    std::string samplerRepeatName = "textureRepeat_sampler";

    PipelineResourceDesc Resources[] =
    {
        {SHADER_TYPE_VERTEX, Prisma::ShaderNames::MUTABLE_MODELS.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_STATUS.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_VERTEX, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1,SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1,SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_LIGHT_SIZES.c_str(), 1,SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_OMNI_DATA.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_SRV,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_DIR_DATA.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_SRV,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_CLUSTERS.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_UAV,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_CLUSTERS_DATA.c_str(), 1,SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str(), 1,SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_OMNI_DATA_SHADOW.c_str(),Define::MAX_OMNI_LIGHTS,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_DIR_SHADOW.c_str(),5,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str(),Define::MAX_MESHES,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str(),Define::MAX_MESHES,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str(),Define::MAX_MESHES,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},

        {SHADER_TYPE_PIXEL,samplerClampName.c_str(),1,SHADER_RESOURCE_TYPE_SAMPLER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL,samplerRepeatName.c_str(),1,SHADER_RESOURCE_TYPE_SAMPLER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::CONSTANT_LUT.c_str(),1,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_PREFILTER.c_str(),1,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_IRRADIANCE.c_str(),1,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},

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

    RefCntAutoPtr<ISampler> samplerClamp;
    RefCntAutoPtr<ISampler> samplerRepeat;

    contextData.m_pDevice->CreatePipelineResourceSignature(ResourceSignDesc, &m_pResourceSignature);

    IPipelineResourceSignature* ppSignatures[]{ m_pResourceSignature };

    PSOCreateInfo.ppResourceSignatures = ppSignatures;
    PSOCreateInfo.ResourceSignaturesCount = _countof(ppSignatures);


    contextData.m_pDevice->CreatePipelineState(PSOCreateInfo, &m_pso);
    contextData.m_pDevice->CreateSampler(SamLinearClampDesc, &samplerClamp);
    contextData.m_pDevice->CreateSampler(SamLinearRepeatDesc, &samplerRepeat);


    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_VERTEX, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(Prisma::MeshHandler::getInstance().viewProjection());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(Prisma::MeshHandler::getInstance().viewProjection());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str())->Set(Prisma::CSMHandler::getInstance().shadowBuffer());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_OMNI_DATA.c_str())->Set(Prisma::LightHandler::getInstance().omniLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_DIR_DATA.c_str())->Set(Prisma::LightHandler::getInstance().dirLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_CLUSTERS.c_str())->Set(Prisma::LightHandler::getInstance().clusters().clusters->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_CLUSTERS_DATA.c_str())->Set(Prisma::LightHandler::getInstance().clusters().clustersData);

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_LIGHT_SIZES.c_str())->Set(Prisma::LightHandler::getInstance().lightSizes());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_LUT.c_str())->Set(Prisma::PipelineLUT::getInstance().lutTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

    IDeviceObject* samplerDeviceClamp = samplerClamp;
    IDeviceObject* samplerDeviceRepeat = samplerRepeat;

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, samplerClampName.c_str())->Set(samplerDeviceClamp);
    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, samplerRepeatName.c_str())->Set(samplerDeviceRepeat);

    // Create a shader resource binding object and bind all static resources in it
    m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);

    m_updateData = [&]()
        {
            auto buffers = Prisma::MeshIndirect::getInstance().modelBuffer();
            auto materials = Prisma::MeshIndirect::getInstance().textureViews();
            auto status = Prisma::MeshIndirect::getInstance().statusBuffer();
            m_srb.Release();
            m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_OMNI_DATA_SHADOW.c_str())->SetArray(Prisma::LightHandler::getInstance().omniData().data(), 0, Prisma::LightHandler::getInstance().omniData().size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_DIR_SHADOW.c_str())->Set(Prisma::LightHandler::getInstance().dirShadowData());
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str())->SetArray(materials.diffuse.data(), 0, materials.diffuse.size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str())->SetArray(materials.normal.data(), 0, materials.normal.size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str())->SetArray(materials.rm.data(), 0, materials.rm.size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, Prisma::ShaderNames::MUTABLE_MODELS.c_str())->Set(buffers->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
            if (status) {
                m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_STATUS.c_str())->Set(status->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
            }
            if (Prisma::PipelineSkybox::getInstance().isInit()) {
                m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_PREFILTER.c_str())->Set(Prisma::PipelinePrefilter::getInstance().prefilterTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
                m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_IRRADIANCE.c_str())->Set(Prisma::PipelineDiffuseIrradiance::getInstance().irradianceTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
            }
        };

    //CreateMSAARenderTarget();
    Prisma::MeshIndirect::getInstance().addResizeHandler([&](Diligent::RefCntAutoPtr<Diligent::IBuffer> buffers, Prisma::MeshIndirect::MaterialView& materials)
        {
            m_updateData();
        });
    Prisma::PipelineSkybox::getInstance().addUpdate([&]()
        {
            m_updateData();
        });
    Prisma::LightHandler::getInstance().addLightHandler([&]()
        {
            m_updateData();
        });
}

void Prisma::PipelineForward::createAnimation()
{
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Forward Pipeline";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PrismaFunc::getInstance().renderFormat().RenderFormat;
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = Prisma::PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
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
    ShaderMacro Macros[] = { {"ANIMATION", "1"} };
    ShaderCI.Macros = { Macros, _countof(Macros) };
    // Pack matrices in row-major order

    // In this tutorial, we will load shaders from file. To be able to do that,
    // we need to create a shader source stream factory
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    contextData.m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Forward VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/ForwardPipeline/vertex.glsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pVS);
    }


    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Forward PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/ForwardPipeline/fragment.glsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pPS);
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


    std::string samplerClampName = "textureClamp_sampler";
    std::string samplerRepeatName = "textureRepeat_sampler";

    PipelineResourceDesc Resources[] =
    {
        {SHADER_TYPE_VERTEX, Prisma::ShaderNames::MUTABLE_MODELS.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_VERTEX, Prisma::ShaderNames::CONSTANT_ANIMATION.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_SRV,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_STATUS.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_VERTEX, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1,SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1,SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_LIGHT_SIZES.c_str(), 1,SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_OMNI_DATA.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_SRV,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_DIR_DATA.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_SRV,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_CLUSTERS.c_str(), 1,SHADER_RESOURCE_TYPE_BUFFER_UAV,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_CLUSTERS_DATA.c_str(), 1,SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str(), 1,SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_OMNI_DATA_SHADOW.c_str(),Define::MAX_OMNI_LIGHTS,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_DIR_SHADOW.c_str(),5,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str(),Define::MAX_MESHES,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str(),Define::MAX_MESHES,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str(),Define::MAX_MESHES,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},

        {SHADER_TYPE_PIXEL,samplerClampName.c_str(),1,SHADER_RESOURCE_TYPE_SAMPLER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL,samplerRepeatName.c_str(),1,SHADER_RESOURCE_TYPE_SAMPLER,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::CONSTANT_LUT.c_str(),1,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_PREFILTER.c_str(),1,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL,Prisma::ShaderNames::MUTABLE_IRRADIANCE.c_str(),1,SHADER_RESOURCE_TYPE_TEXTURE_SRV,SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},

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

    RefCntAutoPtr<ISampler> samplerClamp;
    RefCntAutoPtr<ISampler> samplerRepeat;

    contextData.m_pDevice->CreatePipelineResourceSignature(ResourceSignDesc, &m_pResourceSignatureAnimation);

    IPipelineResourceSignature* ppSignatures[]{ m_pResourceSignatureAnimation };

    PSOCreateInfo.ppResourceSignatures = ppSignatures;
    PSOCreateInfo.ResourceSignaturesCount = _countof(ppSignatures);


    contextData.m_pDevice->CreatePipelineState(PSOCreateInfo, &m_psoAnimation);
    contextData.m_pDevice->CreateSampler(SamLinearClampDesc, &samplerClamp);
    contextData.m_pDevice->CreateSampler(SamLinearRepeatDesc, &samplerRepeat);


    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_VERTEX, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(Prisma::MeshHandler::getInstance().viewProjection());

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_VERTEX, Prisma::ShaderNames::CONSTANT_ANIMATION.c_str())->Set(Prisma::AnimationHandler::getInstance().animation()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(Prisma::MeshHandler::getInstance().viewProjection());

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str())->Set(Prisma::CSMHandler::getInstance().shadowBuffer());

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_OMNI_DATA.c_str())->Set(Prisma::LightHandler::getInstance().omniLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_DIR_DATA.c_str())->Set(Prisma::LightHandler::getInstance().dirLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_CLUSTERS.c_str())->Set(Prisma::LightHandler::getInstance().clusters().clusters->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS));

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_CLUSTERS_DATA.c_str())->Set(Prisma::LightHandler::getInstance().clusters().clustersData);

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_LIGHT_SIZES.c_str())->Set(Prisma::LightHandler::getInstance().lightSizes());

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, Prisma::ShaderNames::CONSTANT_LUT.c_str())->Set(Prisma::PipelineLUT::getInstance().lutTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

    IDeviceObject* samplerDeviceClamp = samplerClamp;
    IDeviceObject* samplerDeviceRepeat = samplerRepeat;

    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, samplerClampName.c_str())->Set(samplerDeviceClamp);
    m_pResourceSignatureAnimation->GetStaticVariableByName(SHADER_TYPE_PIXEL, samplerRepeatName.c_str())->Set(samplerDeviceRepeat);

    // Create a shader resource binding object and bind all static resources in it
    m_pResourceSignatureAnimation->CreateShaderResourceBinding(&m_srbAnimation, true);

    m_updateDataAnimation = [&]()
        {
            auto buffers = Prisma::MeshIndirect::getInstance().modelBufferAnimation();
            auto materials = Prisma::MeshIndirect::getInstance().textureViewsAnimation();
            auto status = Prisma::MeshIndirect::getInstance().statusBufferAnimation();
            m_srbAnimation.Release();
            m_pResourceSignatureAnimation->CreateShaderResourceBinding(&m_srbAnimation, true);
            m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_OMNI_DATA_SHADOW.c_str())->SetArray(Prisma::LightHandler::getInstance().omniData().data(), 0, Prisma::LightHandler::getInstance().omniData().size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_DIR_SHADOW.c_str())->Set(Prisma::LightHandler::getInstance().dirShadowData());
            m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str())->SetArray(materials.diffuse.data(), 0, materials.diffuse.size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str())->SetArray(materials.normal.data(), 0, materials.normal.size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str())->SetArray(materials.rm.data(), 0, materials.rm.size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, Prisma::ShaderNames::MUTABLE_MODELS.c_str())->Set(buffers->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
            if (status) {
                m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_STATUS.c_str())->Set(status->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
            }

            if (Prisma::PipelineSkybox::getInstance().isInit()) {
                m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_PREFILTER.c_str())->Set(Prisma::PipelinePrefilter::getInstance().prefilterTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
                m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, Prisma::ShaderNames::MUTABLE_IRRADIANCE.c_str())->Set(Prisma::PipelineDiffuseIrradiance::getInstance().irradianceTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
            }
        };

    //CreateMSAARenderTarget();
    Prisma::MeshIndirect::getInstance().addResizeHandler([&](Diligent::RefCntAutoPtr<Diligent::IBuffer> buffers, Prisma::MeshIndirect::MaterialView& materials)
        {
            m_updateDataAnimation();
        });
    Prisma::PipelineSkybox::getInstance().addUpdate([&]()
        {
            m_updateDataAnimation();
        });
    Prisma::LightHandler::getInstance().addLightHandler([&]()
        {
            m_updateDataAnimation();
        });
}
