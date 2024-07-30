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
#include "../include/Helpers/IBLBuilder.h"
#include "../include/Helpers/GarbageCollector.h"
#include "../include/Physics/Physics.h"
#include "../include/Physics/DrawDebugger.h"
#include "../include/Handlers/ComponentsHandler.h"
#include "../include/Postprocess/Postprocess.h"
#include "../include/Helpers/Timer.h"
#include <glm/gtx/string_cast.hpp>
#include "../include/SceneData/SceneExporter.h"

std::shared_ptr<Prisma::Engine> Prisma::Engine::instance = nullptr;

struct PrivateData {
    Prisma::PipelineHandler pipelineHandler;
    Prisma::Settings settings;
    std::shared_ptr<Prisma::CallbackHandler> callbackHandler;
    std::shared_ptr<Prisma::Camera> camera;
    Prisma::SceneLoader::SceneParameters sceneParameters;
    Prisma::Engine::Pipeline pipeline;
    Prisma::ComponentsHandler componentsHandler;
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

    SettingsLoader::instance().load(DIR_DEFAULT_SETTINGS);

    PrismaFunc::getInstance();

    LightHandler::getInstance();

    MeshIndirect::getInstance();

    MeshHandler::getInstance();

    IBLBuilder::getInstance();

    PipelineDiffuseIrradiance::getInstance();

    PipelineSkybox::getInstance();

    PipelinePrefilter::getInstance();

    Physics::getInstance();

    Postprocess::getInstance();

    AnimationHandler::getInstance();
    
    data->pipeline = Prisma::Engine::Pipeline::FORWARD;

    data->settings = SettingsLoader::instance().getSettings();

    data->sceneHandler = std::make_shared<Prisma::SceneHandler>();
    
    currentGlobalScene = std::make_shared<Scene>();

    data->lastTime = std::chrono::high_resolution_clock::now();

    data->sceneParameters.srgb = true;

    data->fps = 0.0f;

    data->debug = true;
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
                data->componentsHandler.updateStart();
                data->componentsHandler.updateComponents();
                Physics::getInstance().update(1 / fps());
            }
            data->sceneHandler->onBeginRender();
            data->componentsHandler.updateUi();
            MeshHandler::getInstance().updateCamera();
            MeshHandler::getInstance().updateFragment();
            AnimationHandler::getInstance().updateAnimations();
            MeshIndirect::getInstance().update();
            LightHandler::getInstance().update();

            switch (data->pipeline) {
                case Prisma::Engine::Pipeline::FORWARD:
                    data->pipelineHandler.forward()->render();
                    break;

                case Prisma::Engine::Pipeline::DEFERRED:
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
        Prisma::CacheScene::getInstance().resetCaches();
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

void Prisma::Engine::pipeline(Pipeline pipeline)
{
    data->pipeline = pipeline;
}

Prisma::Engine::Pipeline Prisma::Engine::pipeline()
{
    return data->pipeline;
}

void Prisma::Engine::setCallback(std::shared_ptr<CallbackHandler> callbackHandler)
{
    callbackHandler->resize = [&](int width, int height) {
        currentProjection = glm::perspective(glm::radians(data->settings.angle), (float)data->settings.width / (float)data->settings.height, data->settings.nearPlane, data->settings.farPlane);
        MeshHandler::getInstance().ubo()->modifyData(Prisma::MeshHandler::PROJECTION_OFFSET, sizeof(glm::mat4), glm::value_ptr(currentProjection));
    };
    currentProjection = glm::perspective(glm::radians(data->settings.angle), (float)data->settings.width / (float)data->settings.height, data->settings.nearPlane, data->settings.farPlane);
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

Prisma::Engine& Prisma::Engine::getInstance()
{
    if (!instance) {
        instance = std::make_shared<Engine>();
    }
    return *instance;
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
