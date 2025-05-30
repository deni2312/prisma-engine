#include "Helpers/ClusterCalculation.h"
#include "Helpers/SettingsLoader.h"
#include "GlobalData/GlobalData.h"
#include <glm/gtx/string_cast.hpp>

#include "Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp"
#include "GlobalData/GlobalShaderNames.h"
#include "Handlers/MeshHandler.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"

Prisma::ClusterCalculation::ClusterCalculation(Diligent::RefCntAutoPtr<Diligent::IBuffer> omniLights,
                                               Diligent::RefCntAutoPtr<Diligent::IBuffer> lightSizes) {
    auto& contextData = PrismaFunc::getInstance().contextData();

    m_omniLights = omniLights;
    m_lightSizes = lightSizes;

    Diligent::BufferDesc CBDesc;
    CBDesc.Name = "ClusterData";
    CBDesc.Size = sizeof(ClusterData);
    CBDesc.Usage = Diligent::USAGE_DYNAMIC;
    CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
    contextData.device->CreateBuffer(CBDesc, nullptr, &m_clusterData);

    Diligent::BufferDesc ClusterDesc;
    ClusterDesc.Name = "Cluster";
    ClusterDesc.Usage = Diligent::USAGE_DEFAULT;
    ClusterDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_UNORDERED_ACCESS;
    ClusterDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    ClusterDesc.ElementByteStride = sizeof(Cluster);
    ClusterDesc.Size = sizeof(Cluster) * m_gridSizeX * m_gridSizeY * m_gridSizeZ;
    contextData.device->CreateBuffer(ClusterDesc, nullptr, &m_cluster);
    createCamera();
    createLight();
}

void Prisma::ClusterCalculation::updateCamera() {
    auto currentSettings = SettingsLoader::getInstance().getSettings();
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::MapHelper<ClusterData> clusterData(contextData.immediateContext, m_clusterData,
                                                 Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
    clusterData->gridSize = {m_gridSizeX, m_gridSizeY, m_gridSizeZ, 0};
    clusterData->inverseProjection = inverse(GlobalData::getInstance().currentProjection());
    clusterData->screenDimensions = {currentSettings.width, currentSettings.height, 0, 0};
    clusterData->nearFar.x = GlobalData::getInstance().currentGlobalScene()->camera->nearPlane();
    clusterData->nearFar.y = GlobalData::getInstance().currentGlobalScene()->camera->farPlane();
    Diligent::DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = m_gridSizeX;
    DispatAttribs.ThreadGroupCountY = m_gridSizeY;
    DispatAttribs.ThreadGroupCountZ = m_gridSizeZ;
    contextData.immediateContext->SetPipelineState(m_pso);
    contextData.immediateContext->CommitShaderResources(
        m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->DispatchCompute(DispatAttribs);
}

void Prisma::ClusterCalculation::updateLights() {
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = 27;
    DispatAttribs.ThreadGroupCountY = 1;
    DispatAttribs.ThreadGroupCountZ = 1;
    contextData.immediateContext->SetPipelineState(m_psoLight);
    contextData.immediateContext->CommitShaderResources(
        m_srbLight, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->DispatchCompute(DispatAttribs);
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::ClusterCalculation::clusters() {
    return m_cluster;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::ClusterCalculation::clusterData() {
    return m_clusterData;
}

void Prisma::ClusterCalculation::createCamera() {
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
        ShaderCI.Desc.Name = "Cluster CS";
        ShaderCI.FilePath = "../../../Engine/Shaders/ComputePipeline/compute.glsl";
        contextData.device->CreateShader(ShaderCI, &pResetParticleListsCS);
    }

    Diligent::ComputePipelineStateCreateInfo PSOCreateInfo;
    Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

    // This is a compute pipeline
    PSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;

    PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
	// clang-format off
	Diligent::ShaderResourceVariableDesc Vars[] =
	{
		{Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_CLUSTERS_DATA.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		{ Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_CLUSTERS.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}
	};
    // clang-format on
    PSODesc.ResourceLayout.Variables = Vars;
    PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    PSODesc.Name = "Cluster";
    PSOCreateInfo.pCS = pResetParticleListsCS;
    contextData.device->CreateComputePipelineState(PSOCreateInfo, &m_pso);
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_CLUSTERS_DATA.c_str())->Set(
        m_clusterData);
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_CLUSTERS.c_str())->Set(
        m_cluster->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
    m_pso->CreateShaderResourceBinding(&m_srb, true);
}

void Prisma::ClusterCalculation::createLight() {
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
        ShaderCI.Desc.Name = "Cluster Light CS";
        ShaderCI.FilePath = "../../../Engine/Shaders/ComputePipeline/compute_lights.glsl";
        contextData.device->CreateShader(ShaderCI, &pResetParticleListsCS);
    }

    Diligent::ComputePipelineStateCreateInfo PSOCreateInfo;
    Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

    // This is a compute pipeline
    PSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;

    PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
	// clang-format off
	Diligent::ShaderResourceVariableDesc Vars[] =
	{
		{Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_CLUSTERS.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		{Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_LIGHT_SIZES.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		{ Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		{Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_OMNI_DATA.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
	};
    // clang-format on
    PSODesc.ResourceLayout.Variables = Vars;
    PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    PSODesc.Name = "Cluster Lights";
    PSOCreateInfo.pCS = pResetParticleListsCS;
    contextData.device->CreateComputePipelineState(PSOCreateInfo, &m_psoLight);

    m_psoLight->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_CLUSTERS.c_str())->Set(
        m_cluster->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
    m_psoLight->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_OMNI_DATA.c_str())->
                Set(m_omniLights->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
    m_psoLight->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE,
                                        ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(
        MeshHandler::getInstance().viewProjection());
    m_psoLight->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, ShaderNames::CONSTANT_LIGHT_SIZES.c_str())->
                Set(m_lightSizes);

    m_psoLight->CreateShaderResourceBinding(&m_srbLight, true);
}