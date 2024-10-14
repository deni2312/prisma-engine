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
#include "../include/SceneData/SceneExporter.h"

struct PrivateData {
    Prisma::PipelineHandler pipelineHandler;
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
};

std::shared_ptr<PrivateData> data;

Prisma::Engine::Engine()
{
    data = std::make_shared<PrivateData>();

    SettingsLoader::getInstance().load(DIR_DEFAULT_SETTINGS);

    PrismaFunc::getInstance();

    LightHandler::getInstance();

    MeshIndirect::getInstance();

    MeshHandler::getInstance();

    PrismaRender::getInstance();

    PipelineDiffuseIrradiance::getInstance();

    PipelineSkybox::getInstance();

    PipelinePrefilter::getInstance();
    
    Physics::getInstance();

    Postprocess::getInstance();

    AnimationHandler::getInstance();
    
    data->engineSettings.pipeline = Prisma::EngineSettings::Pipeline::DEFERRED;

    data->engineSettings.ssr = false;

    data->settings = SettingsLoader::getInstance().getSettings();

    data->sceneHandler = std::make_shared<Prisma::SceneHandler>();
    
    currentGlobalScene = std::make_shared<Scene>();

    data->lastTime = std::chrono::high_resolution_clock::now();

    data->sceneParameters.srgb = true;

    data->fps = 0.0f;

    data->debug = true;

    data->camera = std::make_shared<Camera>();
}



bool Prisma::Engine::run()
{
    initScene();
    while (!PrismaFunc::getInstance().shouldClose()) {
        if (data->camera && currentGlobalScene) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> deltaTime = currentTime - data->lastTime;
            data->lastTime = currentTime;
            data->fps = 1.0f / deltaTime.count();
            PrismaFunc::getInstance().clear();
            if (!data->debug) {
                data->userData->update();
                Prisma::ComponentsHandler::getInstance().updateStart();
                Prisma::ComponentsHandler::getInstance().updateComponents();
                Physics::getInstance().update(1.0f / (float)fps());
            }

            data->sceneHandler->onBeginRender();
            if (data->debug) {
                Prisma::ComponentsHandler::getInstance().updateUi();
            }
            MeshHandler::getInstance().updateCamera();
            MeshHandler::getInstance().updateFragment();
            AnimationHandler::getInstance().updateAnimations();
            MeshIndirect::getInstance().update();
            LightHandler::getInstance().update();

            switch (data->engineSettings.pipeline) {
                case Prisma::EngineSettings::Pipeline::FORWARD:
                    data->pipelineHandler.forward()->render();
                    break;

                case Prisma::EngineSettings::Pipeline::DEFERRED:
                    data->pipelineHandler.deferred()->render();
                    break;

            }

            Postprocess::getInstance().render();

            data->sceneHandler->onEndRender();

            PrismaFunc::getInstance().swapBuffers();
        } else {
            std::cerr << "Null camera or scene" << std::endl;
            PrismaFunc::getInstance().closeWindow();
        }
    }
    Prisma::GarbageCollector::getInstance().clear();
    PrismaFunc::getInstance().destroy();
	return true;
}

void Prisma::Engine::setUserEngine(std::shared_ptr<Prisma::UserData> userData) {
    data->userData = userData;
}

void Prisma::Engine::initScene()
{
    data->userData->start();
    MeshHandler::getInstance().updateCluster();
    if (data->pipelineHandler.initScene(data->sceneParameters)) {
        
    }
    else {
        std::cerr << "Null camera or scene" << std::endl;
        PrismaFunc::getInstance().closeWindow();
    }
}

void Prisma::Engine::setGuiData(std::shared_ptr<Prisma::SceneHandler> guiData)
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
    callbackHandler->resize = [&](int width, int height) {
        currentProjection = glm::perspective(glm::radians(currentGlobalScene->camera->angle()), (float)data->settings.width / (float)data->settings.height, currentGlobalScene->camera->nearPlane(), currentGlobalScene->camera->farPlane());
        MeshHandler::getInstance().ubo()->modifyData(Prisma::MeshHandler::PROJECTION_OFFSET, sizeof(glm::mat4), glm::value_ptr(currentProjection));
    };
    currentProjection = glm::perspective(glm::radians(currentGlobalScene->camera->angle()), (float)data->settings.width / (float)data->settings.height, currentGlobalScene->camera->nearPlane(), currentGlobalScene->camera->farPlane());
    MeshHandler::getInstance().ubo()->modifyData(Prisma::MeshHandler::PROJECTION_OFFSET, sizeof(glm::mat4), glm::value_ptr(currentProjection));
    data->callbackHandler = callbackHandler;
    Prisma::PrismaFunc::getInstance().setCallback(callbackHandler);
}
float Prisma::Engine::fps()
{
    return data->fps;
}

void Prisma::Engine::mainCamera(std::shared_ptr<Camera> camera)
{
    data->camera = camera;
    currentGlobalScene->camera = data->camera;
}

void Prisma::Engine::debug(bool debug)
{
    data->debug = debug;
}

std::shared_ptr<Prisma::UserData> Prisma::Engine::getUserEngine()
{
    return data->userData;
}

std::shared_ptr<Prisma::Scene> Prisma::Engine::getScene(std::string scene, Prisma::SceneLoader::SceneParameters sceneParameters)
{
    SceneLoader sceneLoader;
    data->sceneParameters = sceneParameters;
    currentGlobalScene = sceneLoader.loadScene(scene,sceneParameters);
    currentGlobalScene->camera = data->camera;
    MeshIndirect::getInstance().init();
    return currentGlobalScene;
}
