#include "../include/ParticleController.h"

#include "engine.h"
#include "SceneObjects/Sprite.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "GlobalData/GlobalShaderNames.h"
#include "GlobalData/PrismaFunc.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Handlers/LightHandler.h"

void ParticleController::init(std::shared_ptr<Prisma::Node> root,int numParticles)
{
	m_numParticles = numParticles;
	createPointLights(root);
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

	auto spriteFire = std::make_shared<Prisma::Texture>();
	spriteFire->loadTexture({"../../../Resources/DefaultScene/sprites/fire.png", true});

	auto spriteBurst = std::make_shared<Prisma::Texture>();
	spriteBurst->loadTexture({"../../../Resources/DefaultScene/sprites/burst.png", true});

	auto sprite = std::make_shared<Prisma::Sprite>();

	sprite->loadSprites({spriteFire, spriteBurst});
	sprite->numSprites(m_numParticles);
	sprite->size(glm::vec2(0.1f, 0.1f));
	sprite->name("Sprite");
	/*m_compute = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/SpriteCompute/compute.glsl");
	m_compute->use();
	m_deltaPos = m_compute->getUniformPosition("deltaTime");
	m_timePos = m_compute->getUniformPosition("time");*/
	sprite->matrix(glm::translate(glm::mat4(1.0f), glm::vec3(0, -0.5, 0)));
	root->addChild(sprite);

	Diligent::ShaderCreateInfo ShaderCI;
	// Tell the system that the shader source code is in HLSL.
	// For OpenGL, the engine will convert this into GLSL under the hood.
	ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;

	// Create a shader source stream factory to load shaders from files.
	Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
	contextData.m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
	ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;


	Diligent::RefCntAutoPtr<Diligent::IShader> pResetParticleListsCS;
	{
		ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_COMPUTE;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = "Sprite Movement CS";
		ShaderCI.FilePath = "../../../UserEngine/Shaders/SpriteCompute/compute.glsl";
		contextData.m_pDevice->CreateShader(ShaderCI, &pResetParticleListsCS);
	}

	Diligent::BufferDesc CBDesc;
	CBDesc.Name = "TimeData";
	CBDesc.Size = sizeof(TimeData);
	CBDesc.Usage = Diligent::USAGE_DYNAMIC;
	CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
	CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
	contextData.m_pDevice->CreateBuffer(CBDesc, nullptr, &m_time);


	Diligent::ComputePipelineStateCreateInfo PSOCreateInfo;
	Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

	// This is a compute pipeline
	PSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;

	PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
	// clang-format off
	Diligent::ShaderResourceVariableDesc Vars[] =
	{
		{Diligent::SHADER_TYPE_COMPUTE, "SpritesData", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		{ Diligent::SHADER_TYPE_COMPUTE, "SpriteIds", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		{Diligent::SHADER_TYPE_COMPUTE, "TimeData", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		{ Diligent::SHADER_TYPE_COMPUTE, Prisma::ShaderNames::CONSTANT_OMNI_DATA.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}
	};
	// clang-format on
	PSODesc.ResourceLayout.Variables = Vars;
	PSODesc.ResourceLayout.NumVariables = _countof(Vars);

	PSODesc.Name = "Sprite Movement";
	PSOCreateInfo.pCS = pResetParticleListsCS;
	contextData.m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pso);



	m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "TimeData")->Set(m_time);
	m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, Prisma::ShaderNames::CONSTANT_OMNI_DATA.c_str())->Set(Prisma::LightHandler::getInstance().omniLights()->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));

	m_pso->CreateShaderResourceBinding(&m_srb, true);

	m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "SpritesData")->Set(sprite->models()->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));

	m_srb->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "SpriteIds")->Set(sprite->spriteIds()->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
}

void ParticleController::update()
{
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
	if (!m_start)
	{
		m_startPoint = std::chrono::high_resolution_clock::now();
		m_start = true;
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_startPoint).count();
	Diligent::MapHelper<TimeData> timeData(contextData.m_pImmediateContext, m_time, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
	timeData->delta = 1.0f / Prisma::Engine::getInstance().fps();
	timeData->time = static_cast<float>(duration) / 1000.0f;
	timeData->numParticles = m_numParticles;

	Diligent::DispatchComputeAttribs DispatAttribs;
	DispatAttribs.ThreadGroupCountX = m_numParticles;
	DispatAttribs.ThreadGroupCountY = 1;
	DispatAttribs.ThreadGroupCountZ = 1;
	contextData.m_pImmediateContext->SetPipelineState(m_pso);
	contextData.m_pImmediateContext->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	contextData.m_pImmediateContext->DispatchCompute(DispatAttribs);
}

void ParticleController::createPointLights(std::shared_ptr<Prisma::Node> root)
{
	for (int x = 0; x < m_numParticles; ++x) {
		auto light = std::make_shared<Prisma::Light<Prisma::LightType::LightOmni>>();
		Prisma::LightType::LightOmni lightType;
		lightType.diffuse = lightType.diffuse * glm::vec4(8);
		lightType.radius = 2;
		light->type(lightType);
		light->name("PointLight_" + std::to_string(x));
		auto lightParent = std::make_shared<Prisma::Node>();
		lightParent->name("PointParent_" + std::to_string(x));
		lightParent->matrix(glm::mat4(1));
		lightParent->addChild(light);
		root->addChild(lightParent);
	}
}
