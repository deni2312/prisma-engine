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
#include "../include/Handlers/LoadingHandler.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Handlers/TLASHandler.h"
#include "Helpers/ScenePipeline.h"

#include "Helpers/FPSCounter.h"

struct PrivateData {
    Prisma::Settings settings;
    std::shared_ptr<Prisma::CallbackHandler> callbackHandler;
    std::shared_ptr<Prisma::Camera> camera;
    Prisma::SceneLoader::SceneParameters sceneParameters;
    Prisma::EngineSettings::Settings engineSettings;
    std::shared_ptr<Prisma::SceneHandler> sceneHandler;
    std::shared_ptr<Prisma::UserData> userData;
    Prisma::FPSCounter fpsCounter;
    bool debug;
    Prisma::WindowsHelper::WindowsData windowsData;
};

Prisma::Engine::Engine() {
    data = std::make_shared<PrivateData>();

    SettingsLoader::getInstance().load(DIR_DEFAULT_SETTINGS);

    PrismaFunc::getInstance().init();

    MeshIndirect::getInstance();

    MeshHandler::getInstance();

    LightHandler::getInstance();

    Physics::getInstance();

    data->engineSettings.pipeline = EngineSettings::Pipeline::FORWARD;

    data->engineSettings.ssr = false;

    data->engineSettings.ssao = false;

    data->settings = SettingsLoader::getInstance().getSettings();

    GlobalData::getInstance().currentGlobalScene(std::make_shared<Scene>());

    data->sceneParameters.srgb = true;

    data->debug = true;

    data->camera = nullptr;
}

bool Prisma::Engine::run() {
    initScene();
    while (!PrismaFunc::getInstance().shouldClose()) {
        if (data->camera && GlobalData::getInstance().currentGlobalScene()) {
            PrismaFunc::getInstance().bindMainRenderTarget();
            PrismaFunc::getInstance().clear();            

            if (!data->debug) {
                ComponentsHandler::getInstance().updateStart();
                ComponentsHandler::getInstance().updateComponents();
                Physics::getInstance().update(1.0f / fps());
                data->userData->update();
            }

            data->sceneHandler->onBeginRender();
            MeshHandler::getInstance().updateCamera();
            MeshIndirect::getInstance().update();
            TLASHandler::getInstance().update();
            LightHandler::getInstance().update();
            if (data->debug) {
                ComponentsHandler::getInstance().updateUi();
            }

            Prisma::PipelineHandler::getInstance().render(data->engineSettings.pipeline);

            Postprocess::getInstance().render();

            LoadingHandler::getInstance().update(data->camera, data->sceneHandler->onLoading, data->debug);

            data->sceneHandler->onEndRender();

            PrismaFunc::getInstance().poll();

            PrismaFunc::getInstance().update();
            data->fpsCounter.calculate();
        }
    }

    data->userData->finish();
    data->sceneHandler->onDestroy();
    Physics::getInstance().destroy();
    PrismaFunc::getInstance().destroy();
    return true;
}

void Prisma::Engine::setUserEngine(std::shared_ptr<UserData> userData) {
    data->userData = userData;
}

void Prisma::Engine::initScene() {
       
    data->userData->start();
    PipelineHandler::getInstance().initScene(static_cast<bool>(data->sceneHandler));
}

void Prisma::Engine::setGuiData(std::shared_ptr<SceneHandler> guiData) {
    data->sceneHandler = guiData;
}

void Prisma::Engine::engineSettings(const EngineSettings::Settings& engineSettings) {
    data->engineSettings = engineSettings;
}

Prisma::EngineSettings::Settings Prisma::Engine::engineSettings() const {
    return data->engineSettings;
}

void Prisma::Engine::setCallback(std::shared_ptr<CallbackHandler> callbackHandler) {
    auto& contextData = PrismaFunc::getInstance().contextData();
    data->callbackHandler = callbackHandler;
    PrismaFunc::getInstance().setCallback(callbackHandler);
}

float Prisma::Engine::fps() const {
    return data->fpsCounter.getFPS();
}

void Prisma::Engine::mainCamera(const std::shared_ptr<Camera>& camera) {
    data->camera = camera;
    GlobalData::getInstance().currentGlobalScene()->camera = data->camera;
}

void Prisma::Engine::debug(bool debug) {
    data->debug = debug;
}

void Prisma::Engine::windowsData(WindowsHelper::WindowsData windowsData) {
    data->windowsData = windowsData;
}

std::shared_ptr<Prisma::UserData> Prisma::Engine::getUserEngine() {
    return data->userData;
}

void Prisma::Engine::getScene(const std::string& scene,
                              SceneLoader::SceneParameters sceneParameters) {
    LoadingHandler::getInstance().load(scene, sceneParameters);
}