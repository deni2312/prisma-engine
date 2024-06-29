#include "../include/engine.h"
#include "../include/GlobalData/PrismaFunc.h"

#include <iostream>


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



struct PrivateData {
    Prisma::PipelineHandler pipelineHandler;
    Prisma::Settings settings;
    std::shared_ptr<Prisma::ImguiDebug> imguiDebug;
    std::shared_ptr<Prisma::CallbackHandler> callbackHandler;
    std::shared_ptr<Prisma::Camera> camera;
    Prisma::SceneLoader::SceneParameters sceneParameters;
    Prisma::Engine::Pipeline pipeline;
    Prisma::ComponentsHandler componentsHandler;
    Prisma::SceneHandler sceneHandler;
};

std::shared_ptr<PrivateData> data;

Prisma::Engine::Engine(SceneHandler sceneHandler)
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

    data->sceneHandler = sceneHandler;

}

bool Prisma::Engine::update()
{
	return false;
}

bool Prisma::Engine::run()
{
    initScene();
    while (!PrismaFunc::getInstance().shouldClose()) {
        if (data->camera && currentGlobalScene) {

            PrismaFunc::getInstance().clear();
            data->imguiDebug->start();
            data->componentsHandler.updateStart();
            update();
            data->componentsHandler.updateComponents();


            Physics::getInstance().update(1 / fps());

            MeshHandler::getInstance().updateCamera();
            MeshHandler::getInstance().updateFragment();
            AnimationHandler::getInstance().updateAnimations();
            MeshIndirect::getInstance().update();
            LightHandler::getInstance().update();

            switch (data->pipeline) {
                case Prisma::Engine::Pipeline::FORWARD:
                    data->pipelineHandler.forward()->render(data->camera);
                    break;

                case Prisma::Engine::Pipeline::DEFERRED:
                    data->pipelineHandler.deferred()->render(data->camera);
                    break;

            }

            Postprocess::getInstance().render();

            data->imguiDebug->drawGui();

            data->imguiDebug->close();

            PrismaFunc::getInstance().swapBuffers();
        } else {
            std::cerr << "Null camera or scene" << std::endl;
            PrismaFunc::getInstance().closeWindow();
        }
        resetCaches();
    }
    Prisma::GarbageCollector::getInstance().clear();
    PrismaFunc::getInstance().destroy();
	return true;
}

void Prisma::Engine::initScene()
{
    data->imguiDebug = std::make_shared<ImguiDebug>(PrismaFunc::getInstance().window(), data->settings.width, data->settings.height, this);
    MeshHandler::getInstance().updateCluster();
    if (data->pipelineHandler.initScene(data->camera, currentGlobalScene,data->settings,data->sceneParameters,data->imguiDebug)) {
        Postprocess::getInstance().fbo(data->imguiDebug->fbo());
        loadNewScene();
    }
    else {
        std::cerr << "Null camera or scene" << std::endl;
        PrismaFunc::getInstance().closeWindow();
    }
}

void Prisma::Engine::loadNewScene()
{
    MeshIndirect::getInstance().load();
    LightHandler::getInstance().update();
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
        data->pipelineHandler.forward()->projection(glm::perspective(glm::radians(data->settings.angle), (float)data->settings.width / (float)data->settings.height, data->settings.nearPlane, data->settings.farPlane));
        data->pipelineHandler.deferred()->projection(glm::perspective(glm::radians(data->settings.angle), (float)data->settings.width / (float)data->settings.height, data->settings.nearPlane, data->settings.farPlane));
    };
    currentProjection = glm::perspective(glm::radians(data->settings.angle), (float)data->settings.width / (float)data->settings.height, data->settings.nearPlane, data->settings.farPlane);
    data->callbackHandler = callbackHandler;
}
float Prisma::Engine::fps()
{
    return data->imguiDebug->fps();
}

Prisma::Settings Prisma::Engine::settings()
{
    return data->settings;
}

void Prisma::Engine::mainCamera(std::shared_ptr<Camera> camera)
{
    data->camera = camera;
    currentGlobalScene->camera = data->camera;
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
