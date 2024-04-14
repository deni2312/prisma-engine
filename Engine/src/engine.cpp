#include "../include/engine.h"
#include "../include/GlobalData/PrismaFunc.h"

#include <iostream>


#include "../include/SceneData/SceneLoader.h"
#include "../include/GlobalData/Defines.h"
#include "../include/Handlers/LightHandler.h"
#include "../include/Handlers/MeshHandler.h"
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



struct PrivateData {
    Prisma::PipelineHandler pipelineHandler;
    Prisma::Settings settings;
    std::shared_ptr<Prisma::ImguiDebug> imguiDebug;
    std::shared_ptr<Prisma::CallbackHandler> callbackHandler;
    std::shared_ptr<Prisma::Camera> camera;
    Prisma::SceneLoader::SceneParameters sceneParameters;
    Prisma::Engine::Pipeline pipeline;
    Prisma::ComponentsHandler componentsHandler;
    std::shared_ptr<Prisma::ImguiDebug::ImGuiData> imguiData;
    Prisma::Timer timer;
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
    
    data->pipeline = Prisma::Engine::Pipeline::FORWARD;

    data->settings = SettingsLoader::instance().getSettings();

    data->imguiData = std::make_shared<Prisma::ImguiDebug::ImGuiData>();

    data->imguiData->performances.push_back(std::make_shared<std::pair<std::string, float>>("Components",0.0f));

    data->imguiData->performances.push_back(std::make_shared <std::pair<std::string, float>>("Physics", 0.0f));

    data->imguiData->performances.push_back(std::make_shared <std::pair<std::string, float>>("Handlers", 0.0f));

    data->imguiData->performances.push_back(std::make_shared <std::pair<std::string, float>>("Pipelines", 0.0f));

    data->imguiData->performances.push_back(std::make_shared <std::pair<std::string, float>>("Postprocess", 0.0f));

    data->imguiData->performances.push_back(std::make_shared <std::pair<std::string, float>>("ImGui", 0.0f));

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
            data->timer.start();
            PrismaFunc::getInstance().clear();
            data->imguiDebug->start();
            data->componentsHandler.updateStart();
            update();
            data->componentsHandler.updateComponents();
            data->timer.stop();
            data->imguiData->performances.at(0)->second = data->timer.duration_seconds();
            data->timer.start();

            Physics::getInstance().update(1 / fps());

            data->timer.stop();

            data->imguiData->performances.at(1)->second = data->timer.duration_seconds();

            data->timer.start();

            MeshHandler::getInstance().updateCamera();
            MeshIndirect::getInstance().update();
            LightHandler::getInstance().update();

            data->timer.stop();

            data->imguiData->performances.at(2)->second = data->timer.duration_seconds();

            data->timer.start();

            switch (data->pipeline) {
                case Prisma::Engine::Pipeline::FORWARD:
                    data->pipelineHandler.forward()->render(data->camera);
                    break;

                case Prisma::Engine::Pipeline::DEFERRED:
                    data->pipelineHandler.deferred()->render(data->camera);
                    break;

            }

            data->timer.stop();

            data->imguiData->performances.at(3)->second = data->timer.duration_seconds();

            data->timer.start();

            Postprocess::getInstance().render();

            data->timer.stop();

            data->imguiData->performances.at(4)->second = data->timer.duration_seconds();
            
            data->timer.start();

            data->imguiDebug->imguiData(data->imguiData);
            data->timer.start();

            data->imguiDebug->drawGui();

            data->imguiDebug->close();

            data->timer.stop();

            data->imguiData->performances.at(5)->second = data->timer.duration_seconds();

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
    return currentGlobalScene;
}
