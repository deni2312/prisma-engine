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
#include "../include/Containers/FBO.h"
#include "../include/GlobalData/GlobalData.h"
#include "../include/Pipelines/PipelineHandler.h"
#include "../include/Helpers/SettingsLoader.h"
#include "../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../include/Pipelines/PipelinePrefilter.h"
#include "../include/Helpers/PrismaRender.h"
#include "../include/Helpers/GarbageCollector.h"
#include "../include/Physics/Physics.h"
#include "../include/Physics/DrawDebugger.h"
#include "../include/Handlers/ComponentsHandler.h"
#include "../include/Postprocess/Postprocess.h"
#include <glm/gtx/string_cast.hpp>

#include <Windows.h>
#include "ImGuiImplDiligent.hpp"
#include "ImGuiImplWin32.hpp"
#include "../include/Handlers/LoadingHandler.h"

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

std::shared_ptr<PrivateData> data;

std::unique_ptr<Diligent::ImGuiImplDiligent> imguiDiligent;

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

	Diligent::ImGuiDiligentCreateInfo desc;
	desc.pDevice = Prisma::PrismaFunc::getInstance().contextData().m_pDevice;
	imguiDiligent=Diligent::ImGuiImplWin32::Create(Diligent::ImGuiDiligentCreateInfo{ Prisma::PrismaFunc::getInstance().contextData().m_pDevice ,Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetDesc() },(HWND) Prisma::PrismaFunc::getInstance().windowNative());
	

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
			auto contextDesc = Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetDesc();
			PrismaFunc::getInstance().poll();
			PrismaFunc::getInstance().bindMainRenderTarget();
			PrismaFunc::getInstance().clear();
			imguiDiligent->NewFrame(contextDesc.Width, contextDesc.Height, contextDesc.PreTransform);
			auto currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> deltaTime = currentTime - data->lastTime;
			data->lastTime = currentTime;
			data->fps = 1.0f / deltaTime.count();
			data->userData->update();
			/*PrismaFunc::getInstance().clear();
			if (!data->debug)
			{
				data->userData->update();
				ComponentsHandler::getInstance().updateStart();
				ComponentsHandler::getInstance().updateComponents();
				Physics::getInstance().update(1.0f / fps());
			}
			data->sceneHandler->onBeginRender();
			if (data->debug)
			{
				ComponentsHandler::getInstance().updateUi();
			}
			MeshHandler::getInstance().updateCamera();
			MeshHandler::getInstance().updateFragment();
			MeshIndirect::getInstance().update();
			LightHandler::getInstance().update();*/
			//data->sceneHandler->onBeginRender();
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
			}

			//Postprocess::getInstance().render();

			Prisma::LoadingHandler::getInstance().update(data->camera, data->sceneHandler->onLoading);
			//
			//
			//data->sceneHandler->onEndRender();
			//data->sceneHandler->onEndRender();
			/*else
			{
				std::cerr << "Null camera or scene" << std::endl;
				PrismaFunc::getInstance().closeWindow();
			}*/

			imguiDiligent->Render(Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext);
			PrismaFunc::getInstance().update();
		}
	}
	//data->userData->finish();
	//GarbageCollector::getInstance().clear();
	PrismaFunc::getInstance().destroy();
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
	if (!PipelineHandler::getInstance().initScene(data->sceneParameters))
	{
		std::cerr << "Null camera or scene" << std::endl;
		PrismaFunc::getInstance().closeWindow();
	}
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
	callbackHandler->resize = [&](int width, int height)
	{
		Prisma::GlobalData::getInstance().currentProjection(glm::perspective(
			glm::radians(Prisma::GlobalData::getInstance().currentGlobalScene()->camera->angle()),
			static_cast<float>(data->settings.width) / static_cast<float>(data->
			                                                              settings.height),
			Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane(),
			Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane()));
		auto currentProjection = Prisma::GlobalData::getInstance().currentProjection();
		//MeshHandler::getInstance().ubo()->modifyData(sizeof(glm::mat4), sizeof(glm::mat4),value_ptr(currentProjection));
	};
	Prisma::GlobalData::getInstance().currentProjection(glm::perspective(
		glm::radians(Prisma::GlobalData::getInstance().currentGlobalScene()->camera->angle()),
		static_cast<float>(data->settings.width) / static_cast<float>(data->settings.
		                                                                    height),
		Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane(),
		Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane()));
	auto currentProjection = Prisma::GlobalData::getInstance().currentProjection();
	//MeshHandler::getInstance().ubo()->modifyData(sizeof(glm::mat4), sizeof(glm::mat4),value_ptr(currentProjection));
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
