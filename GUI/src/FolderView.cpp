#include "../include/FolderView.h"
#include "../include/ImguiHelper.h"
#include "Handlers/LoadingHandler.h"
#include "Helpers/StringHelper.h"
#include "SceneData/MeshIndirect.h"
#include "../include/TextureInfo.h"
#include "ThirdParty/imgui/imgui.h"


std::string Prisma::GUI::FileBrowser::windowsToString(std::wstring wStr) {
    // Create a codecvt facet for UTF-8 conversion
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    // Convert wstring to UTF-8 string
    std::string utf8Str = converter.to_bytes(wStr);

    // Access the underlying char array
    const char* charStr = utf8Str.c_str();
    return std::string(charStr);
}

void Prisma::GUI::FileBrowser::setPath(const fs::path& path) {
    if (exists(path) && is_directory(path)) {
        m_currentPath = path;
    }
}

void Prisma::GUI::FileBrowser::listDirectoryContents() {
    auto windowSize = ImGui::GetWindowSize();
    int numColumn = 10;

    auto itemSize = ImVec2(100, 100);
    bool isDirectory = false;
    fs::directory_entry entryPath;
    auto data = [&](int i) {
        if (ImGui::BeginTable("FolderTable", numColumn)) {
            for (int j = 0; j < numColumn; j++) {
                if (i * numColumn + j < m_entries.size()) {
                    try {
                        ImGui::TableNextColumn(); // Move to the next column
                        auto entry = m_entries[i * numColumn + j];
                        std::string entryName = entry.path().filename().string();

                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));

                        if (entry.is_directory()) {
                            // Load your folder icon texture here
                            ImGui::ImageButton(
                                m_folder->texture()->GetDefaultView(
                                    Diligent::TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADER_RESOURCE),
                                itemSize);
                            if (ImGui::IsItemClicked()) {
                                isDirectory = true;
                                entryPath = entry;
                            }
                        } else {
                            // Load your file icon texture here
                            ImGui::ImageButton(
                                m_file->texture()->GetDefaultView(
                                    Diligent::TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADER_RESOURCE),
                                itemSize);
                            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                                auto path = windowsToString(entry.path().c_str());

                                if (StringHelper::getInstance().endsWith(
                                    path, ".prisma")) {
                                    if (GlobalData::getInstance().
                                        currentGlobalScene()->root) {
                                        LoadingHandler::getInstance().load(
                                            path, {true, nullptr, true});
                                    } else {
                                        LoadingHandler::getInstance().load(
                                            path, {true, nullptr, false});
                                    }
                                } else {
                                    SceneLoader loader;
                                    auto scene = loader.loadScene(path, {true});
                                    if (GlobalData::getInstance().
                                        currentGlobalScene()->root) {
                                        GlobalData::getInstance().
                                            currentGlobalScene()->root->
                                            addChild(scene->root);
                                    } else {
                                        GlobalData::getInstance().
                                            currentGlobalScene(scene);
                                    }
                                }
                                MeshIndirect::getInstance().init();
                                CacheScene::getInstance().updateSizes(true);
                            }
                        }
                        ImGui::PopStyleColor();

                        bool selected = false;

                        auto textSize = ImGui::CalcTextSize(entryName.c_str());

                        m_fontSize.x = textSize.x;

                        ImGui::Text(entryName.c_str(), selected, 0, m_fontSize);
                    } catch (std::exception& e) {
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::NewLine();
    };
    auto size = m_entries.size();
    if (size % numColumn > 0) {
        size = size / numColumn + 1;
    } else {
        size = size / numColumn;
    }
    ImGuiHelper::getInstance().clipVertical(size, data);

    if (isDirectory) {
        m_currentPath = entryPath.path();
        m_entries.clear();

        addEntries();
    }
}

void Prisma::GUI::FileBrowser::addEntries() {
    for (const auto& entry : fs::directory_iterator(m_currentPath)) {
        auto filter = StringHelper::getInstance().endsWith(entry.path().filename().string(), ".gltf") ||
                      StringHelper::getInstance().endsWith(entry.path().filename().string(), ".prisma");
        if (entry.is_directory() || filter) {
            m_entries.push_back(entry);
        }
    }

    // Custom sort function to ensure folders come first
    std::sort(m_entries.begin(), m_entries.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return a.is_directory() > b.is_directory();
              });
}

Prisma::GUI::FileBrowser::FileBrowser() : m_currentPath(fs::current_path().parent_path().parent_path().parent_path()) {
    m_folder = std::make_shared<Texture>();
    m_folder->loadTexture({"../../../GUI/icons/folder.png", false});

    m_file = std::make_shared<Texture>();
    m_file->loadTexture({"../../../GUI/icons/file.png", false});

    m_back = std::make_shared<Texture>();
    m_back->loadTexture({"../../../GUI/icons/arrow.png", false});
    m_iconSize = glm::vec2(24, 24);
    m_fontSize = m_iconSize;

    addEntries();
}

void Prisma::GUI::FileBrowser::show(unsigned int width, unsigned int height, float offset, float scale,
                                    float translation) {
    ImGui::SetNextWindowPos(ImVec2(0, scale * height + offset));
    ImGui::SetNextWindowSize(ImVec2(width, height - (scale * height + offset)));

    ImGui::Begin("File Browser", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::BeginTabBar("FOLDER VIEW");

    if (ImGui::BeginTabItem("Textures")) {
        TextureInfo::getInstance().showTextures();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Logs")) {
        m_logger.render();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Folders##1")) {
        if (ImGui::ImageButton(
                m_back->texture()->
                        GetDefaultView(Diligent::TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADER_RESOURCE),
                ImVec2(m_iconSize.x, m_iconSize.y)) && m_currentPath.has_parent_path()) {
            m_currentPath = m_currentPath.parent_path();

            m_entries.clear();

            addEntries();
        }
        listDirectoryContents();
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();
}