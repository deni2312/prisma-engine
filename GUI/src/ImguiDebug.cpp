#include "../include/ImGuiDebug.h"
#include "../../Engine/include/Helpers/ScenePrinter.h"
#include "../../Engine/include/GlobalData/PrismaFunc.h"
#include "../../Engine/include/GlobalData/Keys.h"

#include <memory>
#include <Windows.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glm/gtc/type_ptr.hpp"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "../../Engine/include/Helpers/PrismaMath.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/string_cast.hpp"
#include "../../Engine/include/Helpers/IBLBuilder.h"
#include "../../GUI/include/ImGuiStyle.h"
#include "../../Engine/include/Postprocess/Postprocess.h"
#include "../include/TextureInfo.h"
#include "../include/PixelCapture.h"
#include "ImGuizmo.h"
#include "../../Engine/include/SceneData/SceneExporter.h"
#include "../../Engine/include/Helpers/SettingsLoader.h"
#include "../../Engine/include/engine.h"
#include "../../Engine/include/Pipelines/PipelineSkybox.h"

struct PrivateIO {
    ImGuiIO io;
};

std::shared_ptr<Prisma::ImguiDebug> Prisma::ImguiDebug::instance = nullptr;

std::shared_ptr<PrivateIO> data;

Prisma::ImguiDebug::ImguiDebug() : m_fps{60.0f}, m_lastFrameTime{ glfwGetTime() }
{
    data = std::make_shared<PrivateIO>();
    m_camera = std::make_shared<Prisma::Camera>();
    Prisma::Engine::getInstance().mainCamera(m_camera);
    m_imguiCamera.mouseCallback();
    m_imguiCamera.mouseButtonCallback();
    Prisma::Engine::getInstance().setCallback(m_imguiCamera.callback());
    auto settings = Prisma::SettingsLoader::instance().getSettings();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    data->io = ImGui::GetIO();
    data->io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    data->io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    //ImGui::StyleColorsDark();
    Prisma::ImGuiStyles::darkMode();
    ImGui_ImplGlfw_InitForOpenGL(Prisma::PrismaFunc::getInstance().window(), true);
    ImGui_ImplOpenGL3_Init("#version 150");
    Prisma::FBO::FBOData fboData;
    m_height = settings.height;
    m_width = settings.width;
    fboData.width = settings.width;
    fboData.height = settings.height;
    fboData.enableSrgb = true;
    fboData.enableDepth = true;
    m_fbo = std::make_shared<Prisma::FBO>(fboData);
    fboTarget = m_fbo;
    m_shader = std::make_shared<Shader>("../../../GUI/Shaders/Output/vertex.glsl", "../../../GUI/Shaders/Output/fragment.glsl");
    m_shader->use();
    m_bindlessPos = m_shader->getUniformPosition("screenTexture");
    m_modelPos = m_shader->getUniformPosition("model");
    m_scale = 0.72f;
    m_translate = 1.0f - m_scale;
    m_projection = glm::perspective(glm::radians(settings.angle), (float)settings.width / (float)settings.height, settings.nearPlane, settings.farPlane);
    m_model = glm::translate(glm::mat4(1.0f),glm::vec3(0.0f,m_translate,0.0f))*glm::scale(glm::mat4(1.0f), glm::vec3(m_scale));
    m_fileBrowser=std::make_shared<Prisma::FileBrowser>();
    m_effects = std::make_shared<Prisma::Effects>();
    m_effectsBloom = std::make_shared<Prisma::Effects>();
    Prisma::Postprocess::getInstance().addPostProcess(m_effectsBloom);
    Prisma::Postprocess::getInstance().addPostProcess(m_effects);
    Prisma::PixelCapture::getInstance();
    initStatus();
}

void Prisma::ImguiDebug::drawGui()
{
#ifndef NDEBUG
    glDisable(GL_DEPTH_TEST);
    if (!m_run) {
        float windowWidth = m_translate * m_width / 2;
        ImVec2 size;
        auto nextLeft = [&](float pos) {
            ImGui::SetNextWindowPos(ImVec2(0, pos));
            ImGui::SetNextWindowSize(ImVec2(windowWidth, m_height * m_scale - size.y));
        };

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New")) {
                    std::string scene = openFolder();
                    if (scene != "") {
                        Prisma::Engine::getInstance().getScene(scene, { true });
                    }
                }

                if (ImGui::MenuItem("Save")) {

                    auto endsWith = [](std::string const& value, std::string const& ending)
                    {
                        if (ending.size() > value.size()) return false;
                        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
                    };

                    auto prismaScene = endsWith(currentGlobalScene->name, ".prisma");
                    if (prismaScene) {
                        Prisma::Exporter::getInstance().exportScene(currentGlobalScene->name);
                    }
                    else {
                        std::string scene = saveFile();
                        Prisma::Exporter::getInstance().exportScene(scene);
                    }

                }

                if (ImGui::MenuItem("Add model")) {
                    std::string model = openFolder();
                    if (model != "") {
                        Prisma::SceneLoader sceneLoader;
                        auto scene = sceneLoader.loadScene(model, { true });
                        currentGlobalScene->root->addChild(scene->root, false);
                        currentGlobalScene->meshes.insert(currentGlobalScene->meshes.end(), scene->meshes.begin(), scene->meshes.end());
                        if (currentGlobalScene->animateMeshes.size() < MAX_ANIMATION_MESHES) {
                            currentGlobalScene->animateMeshes.insert(currentGlobalScene->animateMeshes.end(), scene->animateMeshes.begin(), scene->animateMeshes.end());
                        }
                        else {
                            std::cerr << "MAX ANIMATION MESHES REACHED" << std::endl;
                        }
                        currentGlobalScene->omniLights.insert(currentGlobalScene->omniLights.end(), scene->omniLights.begin(), scene->omniLights.end());
                        currentGlobalScene->dirLights.insert(currentGlobalScene->dirLights.end(), scene->dirLights.begin(), scene->dirLights.end());

                        Prisma::MeshIndirect::getInstance().init();

                        Prisma::CacheScene::getInstance().updateSizes(true);
                        Prisma::CacheScene::getInstance().skipUpdate(true);
                    }
                }

                if (ImGui::MenuItem("Add skybox")) {
                    std::string scene = openFolder();
                    Prisma::Texture texture;
                    texture.loadEquirectangular(scene);
                    texture.data({ 4096,4096,3 });
                    Prisma::PipelineSkybox::getInstance().texture(texture, true);
                }

                if (ImGui::MenuItem("Textures")) {
                    Prisma::TextureInfo::getInstance().showTextures();
                }

                ImGui::EndMenu();
            }
            size = ImGui::GetWindowSize();
            ImGui::EndMainMenuBar();
        }
        m_initOffset = size.y;

        bool isOpen = true;

        ImGui::SetNextWindowPos(ImVec2(0, m_initOffset));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));

        ImGui::Begin("Dummy Left", &isOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMouseInputs);
        ImGui::Dummy(ImVec2(0.0f, m_height * m_scale));
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(m_width * m_scale + windowWidth, m_initOffset));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));

        ImGui::Begin("Dummy Right", &isOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMouseInputs);
        ImGui::Dummy(ImVec2(0, m_height * m_scale));
        ImGui::End();

        nextLeft(m_initOffset);

        ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::Text(("FPS: " + std::to_string(m_fps)).c_str());
        ImGui::Text("CACHE STATUS:");
        auto stringBool = [](bool data) {return data ? std::string("TRUE") : std::string("FALSE"); };
        ImGui::Text(("    MODEL: " + stringBool(Prisma::CacheScene::getInstance().updateData())).c_str());
        ImGui::Text(("    LIGHT: " + stringBool(Prisma::CacheScene::getInstance().updateLights())).c_str());
        ImGui::Text(("    SIZE: " + stringBool(Prisma::CacheScene::getInstance().updateSizes())).c_str());
        ImGui::Text(("    TEXTURE: " + stringBool(Prisma::CacheScene::getInstance().updateTextures())).c_str());

        if (ImGui::Button("RUN"))
        {
            m_run = !m_run;
        }

        ImGui::Combo("PIPELINE", &m_status.currentitem, m_status.items.data(), m_status.items.size());
        ImGui::Combo("POSTPROCESS", &m_status.currentPostprocess, m_status.postprocess.data(), m_status.postprocess.size());
        ImGui::Checkbox("BLOOM", &m_bloom);
        if (ImGui::Button("Textures"))
        {
            ImGui::OpenPopup("Textures");
        }

        Prisma::TextureInfo::getInstance().showTextures();
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Separator();

        Prisma::ImGuiTabs::getInstance().showNodes(currentGlobalScene->root, 1, m_imguiCamera);
        // Check if the node is clicked
        ImGui::End();

        m_fileBrowser->show(m_width, m_height, m_initOffset, m_scale, m_translate);
        auto currentSelectMesh = dynamic_cast<Mesh*>(m_imguiCamera.currentSelect());
        auto currentSelectLightDir = dynamic_cast<Light<LightType::LightDir>*>(m_imguiCamera.currentSelect());
        auto currentSelectLightOmni = dynamic_cast<Light<LightType::LightOmni>*>(m_imguiCamera.currentSelect());
        MeshInfo::MeshData meshData;
        meshData.camera = m_camera;
        meshData.projection = m_model * m_projection;
        meshData.translate = m_translate;
        meshData.width = m_width;
        meshData.height = m_height;
        meshData.scale = m_scale;
        meshData.initOffset = m_initOffset;

        if (currentSelectMesh) {
            meshData.mesh = currentSelectMesh;
            meshInfo.showSelected(meshData);
        }
        else if (currentSelectLightDir) {
            lightInfo.showSelectedDir(currentSelectLightDir, meshData);
        }
        else if (currentSelectLightOmni) {
            lightInfo.showSelectedOmni(currentSelectLightOmni, meshData);
        }

        updateStatus();
    }
#endif
    drawScene();
    glEnable(GL_DEPTH_TEST);
}

float Prisma::ImguiDebug::fps()
{
    return m_fps;
}

void Prisma::ImguiDebug::start()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void Prisma::ImguiDebug::close()
{
        m_imguiCamera.constraints({ m_translate * m_width / 2,m_initOffset,m_translate * m_width / 2 + m_scale * m_width,m_initOffset + m_height * m_scale,meshInfo.updateMesh(),ImGuizmo::IsOver(),m_scale });
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (!m_run) {
            double currentFrameTime = glfwGetTime();
            if (currentFrameTime != m_lastFrameTime) {  // Avoid division by zero in the first frame
                m_fps = 1.0f / static_cast<float>(currentFrameTime - m_lastFrameTime);
            }
            m_lastFrameTime = currentFrameTime;
            m_imguiCamera.velocity(1.0f / fps());

            m_imguiCamera.updateCamera(m_camera);
            m_imguiCamera.keyboardUpdate(PrismaFunc::getInstance().window());
        }
}

void Prisma::ImguiDebug::imguiData(std::shared_ptr<ImGuiData> data)
{
    m_data = data;
}

Prisma::ImguiDebug& Prisma::ImguiDebug::getInstance()
{
    if (!instance) {
        instance = std::make_shared<ImguiDebug>();
    }
    return *instance;
}

std::shared_ptr<Prisma::SceneHandler> Prisma::ImguiDebug::handlers()
{
    auto handlers = std::make_shared<SceneHandler>();
    handlers->onBeginRender = []() {
        Prisma::ImguiDebug::getInstance().start();
    };
    handlers->onEndRender = []() {
        Prisma::ImguiDebug::getInstance().drawGui();

        Prisma::ImguiDebug::getInstance().close();
    };
    return handlers;
}

std::shared_ptr<Prisma::FBO> Prisma::ImguiDebug::fbo()
{
    return m_fbo;
}

void Prisma::ImguiDebug::drawScene()
{
    m_shader->use();
    m_shader->setInt64(m_bindlessPos, m_fbo->texture());

    glm::mat4 model = glm::mat4(1.0f);
    if (!m_run) {
        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -m_initOffset / (float)m_height, 0.0f)) * m_model;
    }

#ifndef NDEBUG
    m_shader->setMat4(m_modelPos,model);
#else
    m_shader->setMat4(m_modelPos, glm::mat4(1.0f));
#endif
    Prisma::IBLBuilder::getInstance().renderQuad();
}

void Prisma::ImguiDebug::initStatus()
{
    
    m_status.currentitem = 0;
    m_status.currentPostprocess = 0;

    m_status.items.push_back("FORWARD");
    m_status.items.push_back("DEFERRED");

    m_status.postprocess.push_back("NORMAL");
    m_status.postprocess.push_back("SEPPIA");
    m_status.postprocess.push_back("CARTOON");
    m_status.postprocess.push_back("VIGNETTE");
}

std::string Prisma::ImguiDebug::openFolder()
{
    OPENFILENAME ofn;
    char szFile[260];

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    // Open the File Explorer dialog
    if (GetOpenFileName(&ofn) == TRUE) {
        return szFile;
    }
    else {
        return "";
    }
}

std::string Prisma::ImguiDebug::saveFile()
{
    OPENFILENAME ofn;
    char szFile[260];

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.prisma\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    // Open the Save File dialog
    if (GetSaveFileName(&ofn) == TRUE) {
        return szFile;
    }
    else {
        return "";
    }
}

void Prisma::ImguiDebug::updateStatus()
{
    Prisma::Engine::getInstance().pipeline(static_cast<Prisma::Engine::Pipeline>(m_status.currentitem));

    m_effects->effect(static_cast<Prisma::Effects::EFFECTS>(m_status.currentPostprocess));

    if (m_bloom) {
        m_effectsBloom->effect(Prisma::Effects::EFFECTS::BLOOM);
    }
    else {
        m_effectsBloom->effect(Prisma::Effects::EFFECTS::NORMAL);
    }
}
