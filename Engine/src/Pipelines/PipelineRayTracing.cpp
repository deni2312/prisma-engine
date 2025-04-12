#include "Pipelines/PipelineRayTracing.h"

#include "GlobalData/GlobalData.h"
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

#include "GraphicsTypesX.hpp"
#include "TextureUtilities.h"
#include "Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp"
#include "Helpers/UpdateTLAS.h"
#include "../Helpers/UpdateTLAS.cpp"

using namespace Diligent;

Prisma::PipelineRayTracing::PipelineRayTracing(const unsigned int& width, const unsigned int& height, bool srgb) : m_width{
                                                                                                                 width
}, m_height{ height }
{

    m_settings = Prisma::SettingsLoader::getInstance().getSettings();
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
    Diligent::BufferDesc CBDesc;
    CBDesc.Name = "RayTracing Data";
    CBDesc.Size = sizeof(RayTracingData);
    CBDesc.Usage = Diligent::USAGE_DYNAMIC;
    CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;


    contextData.m_pDevice->CreateBuffer(CBDesc, nullptr, &m_raytracingData);

    m_MaxRecursionDepth = std::min(m_MaxRecursionDepth, contextData.m_pDevice->GetAdapterInfo().RayTracing.MaxRecursionDepth);

    // Prepare ray tracing pipeline description.
    RayTracingPipelineStateCreateInfoX PSOCreateInfo;

    PSOCreateInfo.PSODesc.Name = "Ray tracing PSO";
    PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_RAY_TRACING;

    // Define shader macros

    ShaderCreateInfo ShaderCI;
    // We will not be using combined texture samplers as they
    // are only required for compatibility with OpenGL, and ray
    // tracing is not supported in OpenGL backend.
    ShaderCI.Desc.UseCombinedTextureSamplers = false;

    // Only new DXC compiler can compile HLSL ray tracing shaders.
    ShaderCI.ShaderCompiler = SHADER_COMPILER_DXC;

    // Use row-major matrices.
    ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;

    // Shader model 6.3 is required for DXR 1.0, shader model 6.5 is required for DXR 1.1 and enables additional features.
    // Use 6.3 for compatibility with DXR 1.0 and VK_NV_ray_tracing.
    ShaderCI.HLSLVersion = { 6, 3 };
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    // Create a shader source stream factory to load shaders from files.
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    contextData.m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

    // Create ray generation shader.
    RefCntAutoPtr<IShader> pRayGen;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_GEN;
        ShaderCI.Desc.Name = "Ray tracing RG";
        ShaderCI.FilePath = "../../../Engine/Shaders/RayTracingPipeline/raytrace.hlsl";
        ShaderCI.EntryPoint = "main";
        contextData.m_pDevice->CreateShader(ShaderCI, &pRayGen);
        VERIFY_EXPR(pRayGen != nullptr);
    }

    // Create miss shaders.
    RefCntAutoPtr<IShader> pPrimaryMiss;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_MISS;
        ShaderCI.Desc.Name = "Primary ray miss shader";
        ShaderCI.FilePath = "../../../Engine/Shaders/RayTracingPipeline/miss.hlsl";
        ShaderCI.EntryPoint = "main";
        contextData.m_pDevice->CreateShader(ShaderCI, &pPrimaryMiss);
        VERIFY_EXPR(pPrimaryMiss != nullptr);
    }

    // Create closest hit shaders.
    RefCntAutoPtr<IShader> pCubePrimaryHit;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_CLOSEST_HIT;
        ShaderCI.Desc.Name = "Cube primary ray closest hit shader";
        ShaderCI.FilePath = "../../../Engine/Shaders/RayTracingPipeline/hit.hlsl";
        ShaderCI.EntryPoint = "main";
        contextData.m_pDevice->CreateShader(ShaderCI, &pCubePrimaryHit);
        VERIFY_EXPR(pCubePrimaryHit != nullptr);
    }

    // Setup shader groups

    // Ray generation shader is an entry point for a ray tracing pipeline.
    PSOCreateInfo.AddGeneralShader("Main", pRayGen);
    // Primary ray miss shader.
    PSOCreateInfo.AddGeneralShader("PrimaryMiss", pPrimaryMiss);
    // Primary ray hit group for the textured cube.
    PSOCreateInfo.AddTriangleHitShader("CubePrimaryHit", pCubePrimaryHit);

    // Specify the maximum ray recursion depth.
    // WARNING: the driver does not track the recursion depth and it is the
    //          application's responsibility to not exceed the specified limit.
    //          The value is used to reserve the necessary stack size and
    //          exceeding it will likely result in driver crash.
    PSOCreateInfo.RayTracingPipeline.MaxRecursionDepth = static_cast<Uint8>(m_MaxRecursionDepth);

    // Per-shader data is not used.
    PSOCreateInfo.RayTracingPipeline.ShaderRecordSize = 0;

    // Define immutable sampler for g_Texture and g_GroundTexture. Immutable samplers should be used whenever possible
    SamplerDesc SamLinearWrapDesc{
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP //
    };

    PipelineResourceLayoutDescX ResourceLayout;
    ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
    ResourceLayout.AddImmutableSampler(SHADER_TYPE_RAY_CLOSEST_HIT, "g_SamLinearWrap", SamLinearWrapDesc);
    ResourceLayout
        .AddVariable(SHADER_TYPE_RAY_GEN | SHADER_TYPE_RAY_MISS | SHADER_TYPE_RAY_CLOSEST_HIT, "g_ConstantsCB", SHADER_RESOURCE_VARIABLE_TYPE_STATIC)
        .AddVariable(SHADER_TYPE_RAY_GEN, "g_ColorBuffer", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);

    PSOCreateInfo.PSODesc.ResourceLayout = ResourceLayout;

    contextData.m_pDevice->CreateRayTracingPipelineState(PSOCreateInfo, &m_pso);
    VERIFY_EXPR(m_pso != nullptr);

    m_pso->GetStaticVariableByName(SHADER_TYPE_RAY_GEN, "g_ConstantsCB")->Set(m_raytracingData);
    m_pso->GetStaticVariableByName(SHADER_TYPE_RAY_MISS, "g_ConstantsCB")->Set(m_raytracingData);
    m_pso->GetStaticVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "g_ConstantsCB")->Set(m_raytracingData);

    m_pso->CreateShaderResourceBinding(&m_srb, true);
    VERIFY_EXPR(m_srb != nullptr);


    // Create window-size color image.
    TextureDesc RTDesc = {};
    RTDesc.Name = "Color buffer";
    RTDesc.Type = RESOURCE_DIM_TEX_2D;
    RTDesc.Width = m_width;
    RTDesc.Height = m_height;
    RTDesc.BindFlags = BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE;
    RTDesc.ClearValue.Format = TEX_FORMAT_RGBA8_UNORM;
    RTDesc.Format = TEX_FORMAT_RGBA8_UNORM;

    contextData.m_pDevice->CreateTexture(RTDesc, nullptr, &m_colorBuffer);


    Prisma::GlobalData::getInstance().addGlobalTexture({ m_colorBuffer,"RayTracing Color"});

    m_srb->GetVariableByName(SHADER_TYPE_RAY_GEN, "g_ColorBuffer")->Set(m_colorBuffer->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS));

    m_blitRT = std::make_shared<Prisma::PipelineBlitRT>(m_colorBuffer);

    Prisma::UpdateTLAS::getInstance().addUpdates([&](auto vertex, auto primitive, auto index)
        {
            m_srb.Release();
            m_pso->CreateShaderResourceBinding(&m_srb, true);
            m_srb->GetVariableByName(SHADER_TYPE_RAY_GEN, "g_ColorBuffer")->Set(m_colorBuffer->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS));
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_RAY_GEN, "g_TLAS")->Set(Prisma::UpdateTLAS::getInstance().TLAS());
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "vertexBlas")->Set(vertex->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "primitiveBlas")->Set(primitive->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "locationBlas")->Set(index->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
        });

}

void Prisma::PipelineRayTracing::render() {
    if (Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size()) {
        auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

        //mesh->raytracingSrb()->GetVariableByName(Diligent::SHADER_TYPE_RAY_CLOSEST_HIT, "g_TLAS")->Set(Prisma::UpdateTLAS::getInstance().TLAS());
        auto camera = Prisma::GlobalData::getInstance().currentGlobalScene()->camera;
        Diligent::MapHelper<RayTracingData> rayTracingData(contextData.m_pImmediateContext, m_raytracingData, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        rayTracingData->CameraPos = glm::vec4(camera->position(), 1.0);
        rayTracingData->InvViewProj = Prisma::GlobalData::getInstance().currentProjection();
        rayTracingData->InvViewProj = glm::inverse(Prisma::GlobalData::getInstance().currentProjection() * camera->matrix());
        rayTracingData->ClipPlanes = { camera->nearPlane(),camera->farPlane() };
        rayTracingData->MaxRecursion = m_MaxRecursionDepth;
        
        contextData.m_pImmediateContext->SetPipelineState(m_pso);
        contextData.m_pImmediateContext->CommitShaderResources(m_srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        
        TraceRaysAttribs Attribs;
        Attribs.DimensionX = m_colorBuffer->GetDesc().Width;
        Attribs.DimensionY = m_colorBuffer->GetDesc().Height;
        Attribs.pSBT = Prisma::UpdateTLAS::getInstance().SBT();

        contextData.m_pImmediateContext->TraceRays(Attribs);
        m_blitRT->blit();
    }
}

Diligent::RefCntAutoPtr<Diligent::IPipelineState> Prisma::PipelineRayTracing::pso()
{
    return m_pso;
}

Prisma::PipelineRayTracing::~PipelineRayTracing()
{
}

Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> Prisma::PipelineRayTracing::srb() 
{
    return m_srb;
}

Diligent::RefCntAutoPtr<Diligent::ITexture> Prisma::PipelineRayTracing::colorBuffer()
{
    return m_colorBuffer;
}

