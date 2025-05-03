#pragma once
#include <memory>
#include <vector>
#include <string>

#include "ImGuiCamera.h"
#include "GlobalData/InstanceData.h"
#include "Helpers/TimeCounter.h"
#include "SceneData/SceneExporter.h"
#include "FolderView.h"
#include "AddingMenu.h"
#include "MeshInfo.h"
#include "LightInfo.h"
#include "SettingsTab.h"
#include "Imgui/interface/ImGuiImplDiligent.hpp"
#include "QueryGPU.h"


namespace Prisma::GUI {
class ImguiDebug : public InstanceData<ImguiDebug> {
public:
    struct ImGuiData {
        std::vector<std::shared_ptr<std::pair<std::string, float>>> performances;
    };


    struct ImGuiStatus {
        //PIPELINE DATA
        std::vector<const char*> items;
        int currentitem = 0;

        std::vector<const char*> postprocess;
        int currentPostprocess = 0;
    };

    struct GlobalSize {
        float x;
        float y;
    };

    ImguiDebug();
    void drawGui();
    float fps();
    void start();
    void close();
    void imguiData(std::shared_ptr<ImGuiData> data);
    //std::shared_ptr<FBO> fbo();

    std::shared_ptr<SceneHandler> handlers();

    GlobalSize globalSize();

    void scale(float scale);

    float scale();

private:
    double m_lastFrameTime = 0.0;
    float m_fps;
    std::shared_ptr<ImGuiData> m_data;
    //std::shared_ptr<FBO> m_fbo;
    //std::shared_ptr<Shader> m_shader;
    unsigned int m_bindlessPos;
    unsigned int m_modelPos;
    glm::mat4 m_model;

    float m_scale;
    float m_translate;
    unsigned int m_width;
    unsigned int m_height;

    void drawScene();
    void initStatus();
    std::string saveFile();
    void onLoading(std::pair<std::string, int>& data);


    float m_initOffset = 0.0f;

    ImGuiCamera m_imguiCamera;

    std::shared_ptr<Camera> m_camera;

    MeshInfo meshInfo;

    LightInfo lightInfo;

    std::shared_ptr<FileBrowser> m_fileBrowser;

    glm::mat4 m_projection;

    bool m_run = false;

    std::string m_saveLocation;

    std::shared_ptr<Texture> m_runButton;

    std::shared_ptr<Texture> m_pauseButton;

    SettingsTab m_settingsTab;

    bool m_settingsTabShow = false;

    //PlotFPS m_plot;

    TimeCounter m_timeCounterUI;
    TimeCounter m_timeCounterEngine;

    ImGuiAddingMenu m_addingMenu;

    Exporter m_exporter;

    float m_buttonSize = 44;

    std::shared_ptr<SceneHandler> m_handlers;

    GlobalSize m_globalSize;

    std::unique_ptr<Diligent::ImGuiImplDiligent> m_imguiDiligent;
};
}