#include "../include/engine.h"
#include "../include/GlobalData/PrismaFunc.h"

#include <iostream>
#include <chrono>


#include "../include/SceneData/SceneLoader.h"
#include "../include/GlobalData/Defines.h"
#include "../include/Handlers/LightHandler.h"
#include "../include/Handlers/MeshHandler.h"
#include "../include/Handlers/AnimationHandler.h"
#include "../include/SceneData/MeshIndirect.h"
#include "../include/GlobalData/GlobalData.h"
#include "../include/Pipelines/PipelineHandler.h"
#include "../include/Helpers/SettingsLoader.h"
#include "../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../include/Pipelines/PipelinePrefilter.h"
#include "../include/Helpers/PrismaRender.h"
#include "../include/Physics/Physics.h"
#include "../include/Handlers/ComponentsHandler.h"
#include "../include/Postprocess/Postprocess.h"
#include <glm/gtx/string_cast.hpp>

#include <Windows.h>
#include "ImGuiImplDiligent.hpp"
#include "ImGuiImplWin32.hpp"
#include "../include/Handlers/LoadingHandler.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Helpers/UpdateTLAS.h"

struct PrivateData
{
	Prisma::Settings settings;
	std::shared_ptr<Prisma::CallbackHandler> callbackHandler;
	std::shared_ptr<Prisma::Camera> camera;
	Prisma::SceneLoader::SceneParameters sceneParameters;
	Prisma::EngineSettings::Settings engineSettings;
	std::shared_ptr<Prisma::SceneHandler> sceneHandler;
	std::shared_ptr<Prisma::UserData> userData;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
	float fps;
	bool debug;
	Prisma::WindowsHelper::WindowsData windowsData;
};

Prisma::Engine::Engine()
{
	data = std::make_shared<PrivateData>();

	SettingsLoader::getInstance().load(DIR_DEFAULT_SETTINGS);

	/*LightHandler::getInstance();

	MeshIndirect::getInstance();

	MeshHandler::getInstance();

	PrismaRender::getInstance();

	PipelineDiffuseIrradiance::getInstance();

	PipelineSkybox::getInstance();

	PipelinePrefilter::getInstance();

	Physics::getInstance();

	Postprocess::getInstance();

	AnimationHandler::getInstance();*/


	PrismaFunc::getInstance().init();

	MeshIndirect::getInstance();

	MeshHandler::getInstance();

	LightHandler::getInstance();

	Physics::getInstance();

	Diligent::ImGuiDiligentCreateInfo desc;
	desc.pDevice = Prisma::PrismaFunc::getInstance().contextData().m_pDevice;

	data->engineSettings.pipeline = EngineSettings::Pipeline::FORWARD;

	data->engineSettings.ssr = false;

	data->engineSettings.ssao = false;

	data->settings = SettingsLoader::getInstance().getSettings();

	data->sceneHandler = std::make_shared<SceneHandler>();

	Prisma::GlobalData::getInstance().currentGlobalScene(std::make_shared<Scene>());

	data->lastTime = std::chrono::high_resolution_clock::now();

	data->sceneParameters.srgb = true;

	data->fps = 0.0f;

	data->debug = true;

	data->camera = nullptr;
}

bool Prisma::Engine::run()
{
	initScene();
	while (!PrismaFunc::getInstance().shouldClose())
	{
		if (data->camera && Prisma::GlobalData::getInstance().currentGlobalScene()) {
			PrismaFunc::getInstance().bindMainRenderTarget();
			PrismaFunc::getInstance().clear();

			auto currentTime = std::chrono::high_resolution_clock::now();

			std::chrono::duration<float> deltaTime = currentTime - data->lastTime;
			data->lastTime = currentTime;
			data->fps = 1.0f / deltaTime.count();

			if (!data->debug)
			{
				data->userData->update();
				ComponentsHandler::getInstance().updateStart();
				ComponentsHandler::getInstance().updateComponents();
				Physics::getInstance().update(1.0f / fps());
			}

			data->sceneHandler->onBeginRender();
			MeshHandler::getInstance().updateCamera();
			MeshIndirect::getInstance().update();
			UpdateTLAS::getInstance().update();
			LightHandler::getInstance().update();
			if (data->debug)
			{
				ComponentsHandler::getInstance().updateUi();
			}
			/*MeshHandler::getInstance().updateCamera();
			MeshHandler::getInstance().updateFragment();
			MeshIndirect::getInstance().update();
			LightHandler::getInstance().update();*/


			switch (data->engineSettings.pipeline)
			{
			case EngineSettings::Pipeline::FORWARD:
				PipelineHandler::getInstance().forward()->render();
				break;
			case EngineSettings::Pipeline::DEFERRED:
				PipelineHandler::getInstance().deferred()->render();
				break;
			case EngineSettings::Pipeline::DEFERRED_FORWARD:
				PipelineHandler::getInstance().deferredForward()->render();
				break;
			case EngineSettings::Pipeline::RAYTRACING:
				PipelineHandler::getInstance().raytracing()->render();
				break;
			}

			//Postprocess::getInstance().render();

			//
			//
			//data->sceneHandler->onEndRender();
			//data->sceneHandler->onEndRender();
			/*else
			{
				std::cerr << "Null camera or scene" << std::endl;
				PrismaFunc::getInstance().closeWindow();
			}*/
			Prisma::LoadingHandler::getInstance().update(data->camera, data->sceneHandler->onLoading);

			data->sceneHandler->onEndRender();

			PrismaFunc::getInstance().poll();

			PrismaFunc::getInstance().update();
		}
	}
	//data->userData->finish();
	PrismaFunc::getInstance().destroy();
	Physics::getInstance().destroy();
	return true;
}

void Prisma::Engine::setUserEngine(std::shared_ptr<UserData> userData)
{
	data->userData = userData;
}

void Prisma::Engine::initScene()
{
	data->userData->start();
	//MeshHandler::getInstance().updateCluster();
	PipelineHandler::getInstance().initScene();
}

void Prisma::Engine::setGuiData(std::shared_ptr<SceneHandler> guiData)
{
	data->sceneHandler = guiData;
}

void Prisma::Engine::engineSettings(const EngineSettings::Settings& engineSettings)
{
	data->engineSettings = engineSettings;
}

Prisma::EngineSettings::Settings Prisma::Engine::engineSettings() const
{
	return data->engineSettings;
}

void Prisma::Engine::setCallback(std::shared_ptr<CallbackHandler> callbackHandler)
{
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

	callbackHandler->resize = [&](int width, int height)
		{
			Prisma::GlobalData::getInstance().currentProjection(glm::perspective(
				glm::radians(Prisma::GlobalData::getInstance().currentGlobalScene()->camera->angle()),
				static_cast<float>(data->settings.width) / static_cast<float>(data->
					settings.height),
				Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane(),
				Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane()));
		};
	Prisma::GlobalData::getInstance().currentProjection(glm::perspective(
		glm::radians(Prisma::GlobalData::getInstance().currentGlobalScene()->camera->angle()),
		static_cast<float>(data->settings.width) / static_cast<float>(data->settings.
			height),
		Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane(),
		Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane()));
	data->callbackHandler = callbackHandler;
	PrismaFunc::getInstance().setCallback(callbackHandler);
}

float Prisma::Engine::fps() const
{
	return data->fps;
}

void Prisma::Engine::mainCamera(const std::shared_ptr<Camera>& camera)
{
	data->camera = camera;
	Prisma::GlobalData::getInstance().currentGlobalScene()->camera = data->camera;
}

void Prisma::Engine::debug(bool debug)
{
	data->debug = debug;
}

void Prisma::Engine::windowsData(Prisma::WindowsHelper::WindowsData windowsData)
{
	data->windowsData = windowsData;
}

std::shared_ptr<Prisma::UserData> Prisma::Engine::getUserEngine()
{
	return data->userData;
}

void Prisma::Engine::getScene(const std::string& scene,
	SceneLoader::SceneParameters sceneParameters)
{
	Prisma::LoadingHandler::getInstance().load(scene, sceneParameters);
}