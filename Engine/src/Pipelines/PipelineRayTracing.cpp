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

Prisma::PipelineRayTracing::PipelineRayTracing(const unsigned int& width, const unsigned int& height) : m_width{
                                                                                                            width
                                                                                                        }, m_height{height} {
    m_settings = SettingsLoader::getInstance().getSettings();
    auto& contextData = PrismaFunc::getInstance().contextData();
    BufferDesc CBDesc;
    CBDesc.Name = "RayTracing Data";
    CBDesc.Size = sizeof(RayTracingData);
    CBDesc.Usage = USAGE_DYNAMIC;
    CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;

    contextData.device->CreateBuffer(CBDesc, nullptr, &m_raytracingData);

    m_hardwareMaxReflection = std::min(m_MaxRecursionDepth,
                                       contextData.device->GetAdapterInfo().RayTracing.MaxRecursionDepth);
    m_MaxRecursionDepth = m_hardwareMaxReflection;
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
    ShaderCI.CompileFlags |= SHADER_COMPILE_FLAG_ENABLE_UNBOUNDED_ARRAYS;

    // Shader model 6.3 is required for DXR 1.0, shader model 6.5 is required for DXR 1.1 and enables additional features.
    // Use 6.3 for compatibility with DXR 1.0 and VK_NV_ray_tracing.
    ShaderCI.HLSLVersion = {6, 3};
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    // Create a shader source stream factory to load shaders from files.
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    contextData.engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

    // Create ray generation shader.
    RefCntAutoPtr<IShader> pRayGen;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_GEN;
        ShaderCI.Desc.Name = "Ray tracing RG";
        ShaderCI.FilePath = "../../../Engine/Shaders/RayTracingPipeline/raytrace.hlsl";
        ShaderCI.EntryPoint = "main";
        contextData.device->CreateShader(ShaderCI, &pRayGen);
        VERIFY_EXPR(pRayGen != nullptr);
    }

    RefCntAutoPtr<IShader> pShadowMiss;
    ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_MISS;
    ShaderCI.Desc.Name = "Shadow ray miss shader";
    ShaderCI.FilePath = "../../../Engine/Shaders/RayTracingPipeline/shadow_miss.hlsl";
    ShaderCI.EntryPoint = "main";
    contextData.device->CreateShader(ShaderCI, &pShadowMiss);
    VERIFY_EXPR(pShadowMiss != nullptr);

    RefCntAutoPtr<IShader> pGlassPrimaryHit;
    ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_CLOSEST_HIT;
    ShaderCI.Desc.Name = "Glass primary ray closest hit shader";
    ShaderCI.FilePath = "../../../Engine/Shaders/RayTracingPipeline/transparent.hlsl";
    ShaderCI.EntryPoint = "main";
    contextData.device->CreateShader(ShaderCI, &pGlassPrimaryHit);
    VERIFY_EXPR(pGlassPrimaryHit != nullptr);

    // Create miss shaders.
    RefCntAutoPtr<IShader> pPrimaryMiss;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_MISS;
        ShaderCI.Desc.Name = "Primary ray miss shader";
        ShaderCI.FilePath = "../../../Engine/Shaders/RayTracingPipeline/miss.hlsl";
        ShaderCI.EntryPoint = "main";
        contextData.device->CreateShader(ShaderCI, &pPrimaryMiss);
        VERIFY_EXPR(pPrimaryMiss != nullptr);
    }

    // Create closest hit shaders.
    RefCntAutoPtr<IShader> pPrimaryHit;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_RAY_CLOSEST_HIT;
        ShaderCI.Desc.Name = "Primary ray closest hit shader";
        ShaderCI.FilePath = "../../../Engine/Shaders/RayTracingPipeline/hit.hlsl";
        ShaderCI.EntryPoint = "main";
        contextData.device->CreateShader(ShaderCI, &pPrimaryHit);
        VERIFY_EXPR(pPrimaryHit != nullptr);
    }

    // Setup shader groups

    // Ray generation shader is an entry point for a ray tracing pipeline.
    PSOCreateInfo.AddGeneralShader("Main", pRayGen);
    // Primary ray miss shader.
    PSOCreateInfo.AddGeneralShader("PrimaryMiss", pPrimaryMiss);
    // Primary ray hit group for the textured cube.
    PSOCreateInfo.AddTriangleHitShader("PrimaryHit", pPrimaryHit);

    PSOCreateInfo.AddTriangleHitShader("GlassPrimaryHit", pGlassPrimaryHit);

    PSOCreateInfo.AddGeneralShader("ShadowMiss", pShadowMiss);

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

    SamplerDesc SamLinearClampDesc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
    };

    PipelineResourceDesc Resources[] =
    {
        {SHADER_TYPE_RAY_GEN, "g_TLAS", 1, SHADER_RESOURCE_TYPE_ACCEL_STRUCT,
         SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_RAY_CLOSEST_HIT, "g_TLAS", 1, SHADER_RESOURCE_TYPE_ACCEL_STRUCT,
         SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_RAY_CLOSEST_HIT, "vertexBlas", 1, SHADER_RESOURCE_TYPE_BUFFER_SRV,
         SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_RAY_CLOSEST_HIT, "primitiveBlas", 1, SHADER_RESOURCE_TYPE_BUFFER_SRV,
         SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::CONSTANT_LIGHT_SIZES.c_str(), 1,
         SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::CONSTANT_DIR_DATA.c_str(), 1,
         SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::MUTABLE_STATUS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV,
         SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_RAY_CLOSEST_HIT, "locationBlas", 1, SHADER_RESOURCE_TYPE_BUFFER_SRV,
         SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_RAY_GEN | SHADER_TYPE_RAY_MISS | SHADER_TYPE_RAY_CLOSEST_HIT, "g_ConstantsCB", 1,
         SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_RAY_GEN, "g_ColorBuffer", 1, SHADER_RESOURCE_TYPE_TEXTURE_UAV,
         SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_RAY_MISS, "skybox", 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV,
         SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::CONSTANT_OMNI_DATA.c_str(), 1,
         SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_RAY_CLOSEST_HIT, "g_SamLinearWrap", 1, SHADER_RESOURCE_TYPE_SAMPLER,
         SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_RAY_MISS, "skybox_sampler", 1, SHADER_RESOURCE_TYPE_SAMPLER,
         SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_RAY_CLOSEST_HIT, "skybox_sampler", 1, SHADER_RESOURCE_TYPE_SAMPLER,
         SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str(), Define::MAX_MESHES,
         SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,
         PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str(), Define::MAX_MESHES,
         SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,
         PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str(),
         Define::MAX_MESHES, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,
         PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::CONSTANT_LUT.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV,
         SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::MUTABLE_PREFILTER.c_str(), 1,
         SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::MUTABLE_IRRADIANCE.c_str(), 1,
         SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},

    };

    PipelineResourceSignatureDesc ResourceSignDesc;
    ResourceSignDesc.NumResources = _countof(Resources);
    ResourceSignDesc.Resources = Resources;

    contextData.device->CreatePipelineResourceSignature(ResourceSignDesc, &m_pResourceSignature);
    RefCntAutoPtr<ISampler> samplerWrap;
    RefCntAutoPtr<ISampler> samplerClamp;

    IPipelineResourceSignature* ppSignatures[]{m_pResourceSignature};

    PSOCreateInfo.ppResourceSignatures = ppSignatures;
    PSOCreateInfo.ResourceSignaturesCount = _countof(ppSignatures);

    contextData.device->CreateRayTracingPipelineState(PSOCreateInfo, &m_pso);
    contextData.device->CreateSampler(SamLinearWrapDesc, &samplerWrap);
    contextData.device->CreateSampler(SamLinearClampDesc, &samplerClamp);
    IDeviceObject* samplerDeviceWrap = samplerWrap;
    IDeviceObject* samplerDeviceClamp = samplerClamp;

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "g_SamLinearWrap")->Set(
        samplerDeviceWrap);
    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_RAY_MISS, "skybox_sampler")->Set(samplerDeviceClamp);
    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "skybox_sampler")->Set(
        samplerDeviceClamp);

    VERIFY_EXPR(m_pso != nullptr);

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_RAY_GEN, "g_ConstantsCB")->Set(m_raytracingData);
    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_RAY_MISS, "g_ConstantsCB")->Set(m_raytracingData);
    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "g_ConstantsCB")->Set(
        m_raytracingData);
    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT,
                                                  ShaderNames::CONSTANT_LIGHT_SIZES.c_str())->Set(
        LightHandler::getInstance().lightSizes());
    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT,
                                                  ShaderNames::CONSTANT_DIR_DATA.c_str())->Set(
        LightHandler::getInstance().dirLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::CONSTANT_LUT.c_str())->
                          Set(PipelineLUT::getInstance().lutTexture()->GetDefaultView(
                              TEXTURE_VIEW_SHADER_RESOURCE));
    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT,
                                                  ShaderNames::CONSTANT_OMNI_DATA.c_str())->Set(
        LightHandler::getInstance().omniLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);
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

    contextData.device->CreateTexture(RTDesc, nullptr, &m_colorBuffer);

    GlobalData::getInstance().addGlobalTexture({m_colorBuffer, "RayTracing Color"});

    m_srb->GetVariableByName(SHADER_TYPE_RAY_GEN, "g_ColorBuffer")->Set(
        m_colorBuffer->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS));

    m_blitRT = std::make_unique<PipelineBlitRT>(m_colorBuffer);

    m_updateData = [&]() {
        m_srb.Release();
        if (UpdateTLAS::getInstance().vertexData() && MeshIndirect::getInstance().statusBuffer() &&
            PipelineSkybox::getInstance().isInit()) {
            auto materials = MeshIndirect::getInstance().textureViews();
            m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);
            m_srb->GetVariableByName(SHADER_TYPE_RAY_GEN, "g_ColorBuffer")->Set(
                m_colorBuffer->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS));
            m_srb->GetVariableByName(SHADER_TYPE_RAY_GEN, "g_TLAS")->Set(UpdateTLAS::getInstance().TLAS());
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "g_TLAS")->Set(
                UpdateTLAS::getInstance().TLAS());
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "vertexBlas")->Set(
                UpdateTLAS::getInstance().vertexData()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "primitiveBlas")->Set(
                UpdateTLAS::getInstance().primitiveData()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, "locationBlas")->Set(
                UpdateTLAS::getInstance().vertexLocation()->GetDefaultView(
                    BUFFER_VIEW_SHADER_RESOURCE));
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT,
                                     ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str())->SetArray(
                materials.diffuse.data(), 0, materials.diffuse.size(),
                SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT,
                                     ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str())->SetArray(
                materials.normal.data(), 0, materials.normal.size(),
                SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT,
                                     ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str())->SetArray(
                materials.rm.data(), 0, materials.rm.size(), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::MUTABLE_STATUS.c_str())->Set(
                MeshIndirect::getInstance().statusBuffer()->GetDefaultView(
                    BUFFER_VIEW_SHADER_RESOURCE));
            m_srb->GetVariableByName(SHADER_TYPE_RAY_MISS, "skybox")->Set(
                PipelineSkybox::getInstance().skybox()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::MUTABLE_PREFILTER.c_str())->
                   Set(PipelinePrefilter::getInstance().prefilterTexture()->GetDefaultView(
                       TEXTURE_VIEW_SHADER_RESOURCE));
            m_srb->GetVariableByName(SHADER_TYPE_RAY_CLOSEST_HIT, ShaderNames::MUTABLE_IRRADIANCE.c_str())->
                   Set(PipelineDiffuseIrradiance::getInstance().irradianceTexture()->GetDefaultView(
                       TEXTURE_VIEW_SHADER_RESOURCE));
        }
    };

    UpdateTLAS::getInstance().addUpdates([&](auto vertex, auto primitive, auto index) {
        m_updateData();
    });
    MeshIndirect::getInstance().addResizeHandler([&](auto vBuffer, auto iBuffer) {
        m_updateData();
    });
    PipelineSkybox::getInstance().addUpdate([&]() {
        m_updateData();
    });
}

void Prisma::PipelineRayTracing::render() {
    if (GlobalData::getInstance().currentGlobalScene()->meshes.size()) {
        auto& contextData = PrismaFunc::getInstance().contextData();

        //mesh->raytracingSrb()->GetVariableByName(Diligent::SHADER_TYPE_RAY_CLOSEST_HIT, "g_TLAS")->Set(Prisma::UpdateTLAS::getInstance().TLAS());
        auto camera = GlobalData::getInstance().currentGlobalScene()->camera;
        MapHelper<RayTracingData> rayTracingData(contextData.immediateContext, m_raytracingData, MAP_WRITE,
                                                 MAP_FLAG_DISCARD);
        rayTracingData->CameraPos = glm::vec4(camera->position(), 1.0);
        rayTracingData->InvViewProj = GlobalData::getInstance().currentProjection();
        rayTracingData->InvViewProj = inverse(GlobalData::getInstance().currentProjection() * camera->matrix());
        rayTracingData->ClipPlanes = {camera->nearPlane(), camera->farPlane()};
        rayTracingData->MaxRecursion = m_MaxRecursionDepth;
        rayTracingData->MaxRecursionReflection = m_maxRecursionReflection;

        contextData.immediateContext->SetPipelineState(m_pso);
        contextData.immediateContext->
                    CommitShaderResources(m_srb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        TraceRaysAttribs Attribs;
        Attribs.DimensionX = m_colorBuffer->GetDesc().Width;
        Attribs.DimensionY = m_colorBuffer->GetDesc().Height;
        Attribs.pSBT = UpdateTLAS::getInstance().SBT();

        contextData.immediateContext->TraceRays(Attribs);
        m_blitRT->blit();
    }
}

RefCntAutoPtr<IPipelineState> Prisma::PipelineRayTracing::pso() {
    return m_pso;
}

Prisma::PipelineRayTracing::~PipelineRayTracing() {
}

RefCntAutoPtr<IShaderResourceBinding> Prisma::PipelineRayTracing::srb() {
    return m_srb;
}

RefCntAutoPtr<ITexture> Prisma::PipelineRayTracing::colorBuffer() {
    return m_colorBuffer;
}

void Prisma::PipelineRayTracing::maxRecursion(unsigned int recursion) {
    m_MaxRecursionDepth = recursion;
}

Uint32 Prisma::PipelineRayTracing::maxRecursion() {
    return m_MaxRecursionDepth;
}

void Prisma::PipelineRayTracing::maxRecursionReflection(unsigned int recursionReflection) {
    m_maxRecursionReflection = recursionReflection;
}

Uint32 Prisma::PipelineRayTracing::maxRecursionReflection() {
    return m_maxRecursionReflection;
}

Uint32 Prisma::PipelineRayTracing::hardwareMaxReflection() {
    return m_hardwareMaxReflection;
}