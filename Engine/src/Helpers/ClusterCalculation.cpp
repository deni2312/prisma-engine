#include "../../include/Helpers/ClusterCalculation.h"
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/GlobalData/GlobalData.h"
#include <glm/gtx/string_cast.hpp>

#include "Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp"
#include "../../include/GlobalData/GlobalShaderNames.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"

Prisma::ClusterCalculation::ClusterCalculation()
{
	//m_shader = std::make_shared<Shader>("../../../Engine/Shaders/ComputePipeline/compute.glsl");
	//m_shaderLights = std::make_shared<Shader>("../../../Engine/Shaders/ComputePipeline/compute_lights.glsl");
	//m_ssbo = std::make_shared<SSBO>(5);
	//m_ssbo->resize(numClusters * sizeof(Cluster), GL_STATIC_DRAW);
	//m_shader->use();
	//m_nearPos = m_shader->getUniformPosition("zNear");
	//m_farPos = m_shader->getUniformPosition("zFar");
	//m_inverseProjectionPos = m_shader->getUniformPosition("inverseProjection");
	//m_gridSizePos = m_shader->getUniformPosition("gridSize");
	//m_screenDimensionsPos = m_shader->getUniformPosition("screenDimensions");
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

	Diligent::ShaderCreateInfo ShaderCI;
	// Tell the system that the shader source code is in HLSL.
	// For OpenGL, the engine will convert this into GLSL under the hood.
	ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;

	// OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
	ShaderCI.Desc.UseCombinedTextureSamplers = true;

	// Create a shader source stream factory to load shaders from files.
	Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
	contextData.m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;


	Diligent::RefCntAutoPtr<Diligent::IShader> pResetParticleListsCS;
	{
		ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_COMPUTE;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Cluster CS";
		ShaderCI.FilePath = "../../../Engine/Shaders/ComputePipeline/compute.hlsl";
		contextData.m_pDevice->CreateShader(ShaderCI, &pResetParticleListsCS);
	}

	Diligent::BufferDesc CBDesc;
	CBDesc.Name = "ClusterData";
	CBDesc.Size = sizeof(ClusterData);
	CBDesc.Usage = Diligent::USAGE_DYNAMIC;
	CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
	CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
	contextData.m_pDevice->CreateBuffer(CBDesc, nullptr, &m_clusterData);

	Diligent::BufferDesc ClusterDesc;
	ClusterDesc.Name = "Cluster";
	ClusterDesc.Usage = Diligent::USAGE_DEFAULT;
	ClusterDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_UNORDERED_ACCESS;
	ClusterDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
	ClusterDesc.ElementByteStride = sizeof(Cluster);
	ClusterDesc.Size = sizeof(Cluster)*m_gridSizeX*m_gridSizeY*m_gridSizeZ;
	contextData.m_pDevice->CreateBuffer(ClusterDesc, nullptr, &m_cluster);

	Diligent::ComputePipelineStateCreateInfo PSOCreateInfo;
	Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

	// This is a compute pipeline
	PSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;

	PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
	// clang-format off
	Diligent::ShaderResourceVariableDesc Vars[] =
	{
		{Diligent::SHADER_TYPE_COMPUTE, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		{ Diligent::SHADER_TYPE_COMPUTE, "clusters", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
	};
	// clang-format on
	PSODesc.ResourceLayout.Variables = Vars;
	PSODesc.ResourceLayout.NumVariables = _countof(Vars);

	PSODesc.Name = "Cluster";
	PSOCreateInfo.pCS = pResetParticleListsCS;
	contextData.m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pso);
	m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Constants")->Set(m_clusterData);
	m_pso->CreateShaderResourceBinding(&m_srb, true);
	m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "clusters")->Set(m_cluster->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
}

void Prisma::ClusterCalculation::updateCamera()
{
	auto currentSettings = SettingsLoader::getInstance().getSettings();
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

	Diligent::MapHelper<ClusterData> clusterData(contextData.m_pImmediateContext, m_clusterData, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
	clusterData->gridSize = { m_gridSizeX, m_gridSizeY, m_gridSizeZ,0 };
	clusterData->inverseProjection = glm::inverse(Prisma::GlobalData::getInstance().currentProjection());
	clusterData->screenDimensions = { currentSettings.width, currentSettings.height,0,0 };
	clusterData->nearFar.x = Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane();
	clusterData->nearFar.y = Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane();
	Diligent::DispatchComputeAttribs DispatAttribs;
	DispatAttribs.ThreadGroupCountX = m_gridSizeX;
	DispatAttribs.ThreadGroupCountY = m_gridSizeY;
	DispatAttribs.ThreadGroupCountZ = m_gridSizeZ;
	contextData.m_pImmediateContext->SetPipelineState(m_pso);
	contextData.m_pImmediateContext->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	contextData.m_pImmediateContext->DispatchCompute(DispatAttribs);
	//m_shader->use();
	//m_shader->setFloat(m_nearPos, Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane());
	//m_shader->setFloat(m_farPos, Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane());
	//m_shader->setMat4(m_inverseProjectionPos, glm::inverse(Prisma::GlobalData::getInstance().currentProjection()));
	//m_shader->setUVec3(m_gridSizePos, {m_gridSizeX, m_gridSizeY, m_gridSizeZ});
	//m_shader->setUVec2(m_screenDimensionsPos, {currentSettings.width, currentSettings.height});
	//m_shader->dispatchCompute({m_gridSizeX, m_gridSizeY, m_gridSizeZ});
	//m_shader->wait(GL_SHADER_STORAGE_BARRIER_BIT);
}

void Prisma::ClusterCalculation::updateLights()
{
	//m_shaderLights->use();
	//m_shaderLights->dispatchCompute({27, 1, 1});
	//m_shaderLights->wait(GL_SHADER_STORAGE_BARRIER_BIT);
}
