#include "GlobalData/GlobalData.h"
#include "Helpers/SettingsLoader.h"
#include "ImGuiDebug.h"
#include "ImGuiStyle.h"
#include "ImGuiKey.h"
#include "ThirdParty/imgui/imgui.h"
#include "SceneObjects/Camera.h"
#include <Windows.h>

#include "engine.h"
#include "Handlers/LoadingHandler.h"
#include "Helpers/StringHelper.h"
#include "Pipelines/PipelineSkybox.h"
#include "SceneData/MeshIndirect.h"
#include "ImGuiTabs.h"
#include "NodeViewer.h"
#include "Imgui/interface/ImGuiImplWin32.hpp"
#include "../imguizmo/imguizmo.h"
#include "PixelCapture.h"
#include "Helpers/ScenePipeline.h"

struct PrivateIO {
    ImGuiIO io;
};

std::shared_ptr<PrivateIO> data;

Prisma::GUI::ImguiDebug::ImguiDebug() : m_lastFrameTime{glfwGetTime()}, m_fps{60.0f} {
    auto& contextData = PrismaFunc::getInstance().contextData();

    m_imguiDiligent = Diligent::ImGuiImplWin32::Create(
        Diligent::ImGuiDiligentCreateInfo{contextData.device, contextData.swapChain->GetDesc()},
        static_cast<HWND>(PrismaFunc::getInstance().windowNative()));

    data = std::make_shared<PrivateIO>();
    m_camera = std::make_shared<Camera>();
    Engine::getInstance().mainCamera(m_camera);
    PrismaFunc::UIInput uiInput;
    uiInput.mouseClick = [&](int button, int action) {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseButtonEvent(button, action);
    };
    uiInput.keyboard = [&](int key, int action, int scancode) {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(convertImGuiGlfwKey(key, scancode), (action == GLFW_PRESS));
        if (action == GLFW_PRESS && key >= 32 && key <= 126) {
            io.AddInputCharacter(key);
        }
    };

    uiInput.mouseRoll = [&](int x, int y) {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMouseWheelEvent(x, y);
    };

    PrismaFunc::getInstance().inputUI(uiInput);
    m_imguiCamera.mouseCallback();
    m_imguiCamera.mouseButtonCallback();
    Engine::getInstance().setCallback(m_imguiCamera.callback());
    auto settings = SettingsLoader::getInstance().getSettings();
    m_globalSize.x = settings.width / 1920.0f;
    m_globalSize.y = settings.height / 1080.0f;

    //ImPlot::CreateContext();
    data->io = ImGui::GetIO();
    data->io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    data->io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    //ImGui::StyleColorsDark();
    ImGuiStyles::getInstance().darkMode();
    m_height = settings.height;
    m_width = settings.width;
    m_scale = 0.72f;
    m_translate = 1.0f - m_scale;
    m_projection = glm::perspective(
        glm::radians(GlobalData::getInstance().currentGlobalScene()->camera->angle()),
        static_cast<float>(settings.width) / static_cast<float>(settings.height),
        GlobalData::getInstance().currentGlobalScene()->camera->nearPlane(),
        GlobalData::getInstance().currentGlobalScene()->camera->farPlane());

    m_model = translate(glm::mat4(1.0f), glm::vec3(0.0f, m_translate, 0.0f)) * glm::scale(
                  glm::mat4(1.0f), glm::vec3(m_scale));

    m_fileBrowser = std::make_shared<FileBrowser>();

    m_runButton = std::make_shared<Texture>();
    m_runButton->loadTexture({"../../../GUI/icons/run.png", false});

    m_pauseButton = std::make_shared<Texture>();
    m_pauseButton->loadTexture({"../../../GUI/icons/pause.png", false});

    NodeViewer::getInstance();
    ScenePipeline::getInstance();
    PixelCapture::getInstance();
    initStatus();
}

void Prisma::GUI::ImguiDebug::drawGui() {
    m_translate = 1.0f - m_scale;
    float windowWidth = m_translate * m_width / 2;
    ImVec2 size;
    auto nextLeft = [&](float pos) {
        ImGui::SetNextWindowPos(ImVec2(0, pos));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, m_height * m_scale - size.y));
    };
    bool openSettings = false;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                auto children = GlobalData::getInstance().currentGlobalScene()->root->children();
                for (const auto& child : children) {
                    GlobalData::getInstance().currentGlobalScene()->root->
                                              removeChild(child->uuid());
                }
                m_imguiCamera.currentSelect(nullptr);
            }

            if (ImGui::MenuItem("Save")) {
                std::string scene = saveFile();
                m_exporter.exportScene(scene);
            }

            if (ImGui::MenuItem("Add model")) {
                std::string model = WindowsHelper::getInstance().openFolder("All Files");
                if (model != "") {
                    if (StringHelper::getInstance().endsWith(model, ".prisma")) {
                        if (GlobalData::getInstance().currentGlobalScene()->root) {
                            LoadingHandler::getInstance().
                                load(model, {true, nullptr, true});
                        } else {
                            LoadingHandler::getInstance().load(
                                model, {true, nullptr, false});
                        }
                    } else {
                        SceneLoader loader;
                        auto scene = loader.loadScene(model, {true});
                        if (GlobalData::getInstance().currentGlobalScene()->root) {
                            GlobalData::getInstance().currentGlobalScene()->root->addChild(
                                scene->root);
                        } else {
                            GlobalData::getInstance().currentGlobalScene(scene);
                        }
                    }
                    MeshIndirect::getInstance().init();
                }
            }

            if (ImGui::MenuItem("Add skybox")) {
                std::string scene = WindowsHelper::getInstance().openFolder("All Files");
                if (scene != "") {
                    Texture texture;
                    texture.loadTexture({scene, true});
                    PipelineSkybox::getInstance().texture(texture);
                }
            }

            if (ImGui::MenuItem("Settings")) {
                openSettings = true;
            }

            if (ImGui::MenuItem("Close")) {
                PrismaFunc::getInstance().closeWindow();
            }

            ImGui::EndMenu();
        }
        size = ImGui::GetWindowSize();
        ImGui::EndMainMenuBar();
    }
    m_initOffset = size.y;
    bool isOpen = true;
    if (!m_run) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth, m_initOffset));
        ImGui::SetNextWindowSize(ImVec2(m_width * m_scale, 0));
    } else {
        ImGui::SetNextWindowPos(ImVec2(0, m_initOffset));
        ImGui::SetNextWindowSize(ImVec2(m_width, 0));
    }
    ImGui::Begin("Dummy Top", &isOpen,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoCollapse);

    auto positionRun = m_run ? m_width / 2 : m_width * m_scale / 2;

    auto& query = QueryGPU::getInstance().queryData();

    if (ImGui::Button("Show Stats", ImVec2(0, 36))) {
        // Button logic if needed
    }
    ImGui::SameLine();
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip(); // Start tooltip when hovering

        if (ImGui::BeginTable("StatsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            auto addRow = [](const char* label, auto valueFormat, auto&& value) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(label);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text(valueFormat, std::forward<decltype(value)>(value));
            };
            addRow("Engine Time:", "%.9f", m_timeCounterEngine.duration_seconds());
            addRow("FPS:", "%.2f", Prisma::Engine::getInstance().fps());
            addRow("Meshes:", "%zu", GlobalData::getInstance().currentGlobalScene()->meshes.size());
            addRow("Animate Meshes:", "%zu", GlobalData::getInstance().currentGlobalScene()->animateMeshes.size());
            addRow("Omni Lights:", "%zu", GlobalData::getInstance().currentGlobalScene()->omniLights.size());
            addRow("Dir Lights:", "%zu", GlobalData::getInstance().currentGlobalScene()->dirLights.size());
            addRow("Area Lights:", "%zu", GlobalData::getInstance().currentGlobalScene()->areaLights.size());
            addRow("Input Vertices:", "%d", query.PipelineStats.InputVertices);
            addRow("Input Primitives:", "%d", query.PipelineStats.InputPrimitives);
            addRow("VS Invocations:", "%d", query.PipelineStats.VSInvocations);
            addRow("Clipping Invocations:", "%d", query.PipelineStats.ClippingInvocations);
            addRow("Clipping Primitives:", "%d", query.PipelineStats.ClippingPrimitives);
            addRow("PS Invocations:", "%d", query.PipelineStats.PSInvocations);
            addRow("Occlusion:", "%d", query.Occlusion.NumSamples);

            ImGui::EndTable();
        }

        ImGui::EndTooltip(); // End tooltip
    }

    ImGui::SetCursorPosX(positionRun);

    auto currentButton = m_run ? m_pauseButton : m_runButton;
    if (ImGui::ImageButton(
        currentButton->texture()->GetDefaultView(Diligent::TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADER_RESOURCE),
        ImVec2(24, 24))) {
        m_run = !m_run;
        Engine::getInstance().debug(!m_run);
        if (m_run) {
            Engine::getInstance().setCallback(Engine::getInstance().getUserEngine()->callbacks());
        } else {
            CacheScene::getInstance().updateSizes(true);
            Engine::getInstance().setCallback(m_imguiCamera.callback());
        }
    }
    m_buttonSize = ImGui::GetWindowSize().y;

    ImGui::End();
    m_model = translate(glm::mat4(1.0f),
                        glm::vec3(
                            0.0f, m_translate - 2 * (m_buttonSize + m_initOffset) / static_cast<float>(
                                      m_height), 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(m_scale));

    if (!m_run) {
        ImGui::SetNextWindowPos(ImVec2(0, m_initOffset));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));

        ImGui::Begin("Dummy Left", &isOpen,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMouseInputs);
        ImGui::Dummy(ImVec2(0.0f, m_height * m_scale + m_buttonSize));
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(m_width * m_scale + windowWidth, m_initOffset));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));

        ImGui::Begin("Dummy Right", &isOpen,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMouseInputs);
        ImGui::Dummy(ImVec2(0, m_height * m_scale + m_buttonSize));
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(0, m_initOffset));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, m_height * m_scale + m_buttonSize));
        ImGui::Begin("Scene", nullptr,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_HorizontalScrollbar);
        //m_plot.showFPS(m_fps);

        if (openSettings) {
            ImGui::OpenPopup("SettingsTab");
            openSettings = false;
        }
        m_settingsTab.drawSettings();
        ImGuiTabs::getInstance().showNodes(GlobalData::getInstance().currentGlobalScene()->root, m_imguiCamera);
        // Check if the node is clicked
        ImGui::End();
        m_fileBrowser->show(m_width, m_height, m_initOffset + m_buttonSize, m_scale, m_translate);
        if (m_imguiCamera.currentSelect()) {
            auto currentSelectMesh = std::dynamic_pointer_cast<Mesh>(m_imguiCamera.currentSelect());
            auto currentSelectLightDir = std::dynamic_pointer_cast<Light<LightType::LightDir>>(
                m_imguiCamera.currentSelect());
            auto currentSelectLightOmni = std::dynamic_pointer_cast<Light<LightType::LightOmni>>(
                m_imguiCamera.currentSelect());
            auto currentSelectLightArea = std::dynamic_pointer_cast<Light<LightType::LightArea>>(
                m_imguiCamera.currentSelect());
            NodeViewer::NodeData nodeData;
            nodeData.camera = m_camera;
            nodeData.projection = m_model * m_projection;
            nodeData.translate = m_translate;
            nodeData.width = m_width;
            nodeData.height = m_height;
            nodeData.scale = m_scale;
            nodeData.initOffset = m_initOffset;
            nodeData.node = m_imguiCamera.currentSelect();

            if (currentSelectMesh) {
                meshInfo.showSelected(nodeData);
            } else if (currentSelectLightDir) {
                lightInfo.showSelectedDir(currentSelectLightDir, nodeData);
            } else if (currentSelectLightOmni) {
                lightInfo.showSelectedOmni(currentSelectLightOmni, nodeData);
            } else if (currentSelectLightArea) {
                lightInfo.showSelectedArea(currentSelectLightArea, nodeData);
            } else {
                NodeViewer::getInstance().showSelected(nodeData);
            }
        }

        m_settingsTab.updateStatus();
    }
    drawScene();
    //glEnable(GL_DEPTH_TEST);
}

float Prisma::GUI::ImguiDebug::fps() {
    return m_fps;
}

void Prisma::GUI::ImguiDebug::start() {
    //ImGui_ImplOpenGL3_NewFrame();
    //ImGui_ImplGlfw_NewFrame();
    //
    auto contextDesc = PrismaFunc::getInstance().contextData().swapChain->GetDesc();
    m_imguiDiligent->NewFrame(m_width, m_height, contextDesc.PreTransform);
    ImGuizmo::BeginFrame();
}

void Prisma::GUI::ImguiDebug::close() {
    m_imguiCamera.constraints({
        m_translate * m_width / 2, m_initOffset + 50, m_translate * m_width / 2 + m_scale * m_width,
        m_height * m_scale,
        false, m_scale, m_model
    });

    if (!m_run) {
        double currentFrameTime = glfwGetTime();
        if (currentFrameTime != m_lastFrameTime) {
            // Avoid division by zero in the first frame
            m_fps = 1.0f / static_cast<float>(currentFrameTime - m_lastFrameTime);
        }
        m_lastFrameTime = currentFrameTime;

        m_imguiCamera.updateCamera(m_camera);
        m_imguiCamera.keyboardUpdate(PrismaFunc::getInstance().window());
        m_addingMenu.addMenu(m_imguiCamera);
        ImGuiTabs::getInstance().updateTabs(GlobalData::getInstance().currentGlobalScene()->root, 0);
    }
    m_imguiDiligent->Render(PrismaFunc::getInstance().contextData().immediateContext);
    //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Prisma::GUI::ImguiDebug::imguiData(std::shared_ptr<ImGuiData> data) {
    m_data = data;
}

std::shared_ptr<Prisma::SceneHandler> Prisma::GUI::ImguiDebug::handlers() {
    m_handlers = std::make_shared<SceneHandler>();
    m_handlers->onBeginRender = [&]() {
        m_timeCounterEngine.start();
        getInstance().start();
        QueryGPU::getInstance().start();
    };
    m_handlers->onLoading = [&](auto data) {
        getInstance().onLoading(data);
    };
    m_handlers->onEndRender = [&]() {
        QueryGPU::getInstance().end();
        getInstance().drawGui();
        getInstance().close();
        m_timeCounterEngine.stop();
    };
    m_handlers->onDestroy = [&]() {
        auto* p = m_imguiDiligent.release();
        delete p;
    };
    return m_handlers;
}

Prisma::GUI::ImguiDebug::GlobalSize Prisma::GUI::ImguiDebug::globalSize() {
    return m_globalSize;
}

void Prisma::GUI::ImguiDebug::scale(float scale) {
    m_scale = scale;
}

float Prisma::GUI::ImguiDebug::scale() {
    return m_scale;
}

//
//std::shared_ptr<Prisma::FBO> Prisma::ImguiDebug::fbo()
//{
//	return m_fbo;
//}

void Prisma::GUI::ImguiDebug::drawScene() {
    auto model = glm::mat4(1.0f);
    if (!m_run) {
        model = m_model;
    }
    auto& contextData = PrismaFunc::getInstance().contextData();

    auto pRTV = contextData.swapChain->GetCurrentBackBufferRTV();
    auto pDSV = contextData.swapChain->GetDepthBufferDSV();
    Prisma::ScenePipeline::getInstance().render(model, pRTV, pDSV);
}

void Prisma::GUI::ImguiDebug::initStatus() {
    m_settingsTab.init();
}

std::string Prisma::GUI::ImguiDebug::saveFile() {
    OPENFILENAME ofn;
    char szFile[260];

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.prisma\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    // Open the Save File dialog
    if (GetSaveFileName(&ofn) == TRUE) {
        return szFile;
    }
    return "";
}

void Prisma::GUI::ImguiDebug::onLoading(std::pair<std::string, int>& data) {
    // Begin the popup
    ImGui::OpenPopup("Loading...");

    if (ImGui::BeginPopupModal("Loading...", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        float progress = 0;
        // Display the name of the task being loaded
        if (data.first.empty()) {
            ImGui::Text("Reading prisma");
        } else {
            ImGui::Text("Loading: %s", data.first.c_str());
            progress = static_cast<float>(data.second) / 100.0f; // Convert percentage to fraction
        }
        ImGui::ProgressBar(progress, ImVec2(300, 0)); // ProgressBar(width, height)
        ImGui::EndPopup();
    }
}