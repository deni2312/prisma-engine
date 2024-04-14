#include <filesystem>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include "../../Engine/include/Containers/Texture.h"
#include "imgui.h"
#include "../../Engine/include/SceneData/SceneLoader.h"
#include "../../Engine/include/GlobalData/GlobalData.h"

namespace Prisma
{
    namespace fs = std::filesystem;

    class FileBrowser
    {
    private:

        ImVec2 m_fontSize;
        ImVec2 m_iconSize;

        fs::path currentPath;
        std::shared_ptr<Prisma::Texture> m_folder;
        std::shared_ptr<Prisma::Texture> m_file;
        std::shared_ptr<Prisma::Texture> m_back;

        std::string windowsToString(std::wstring wStr) {
            // Create a codecvt facet for UTF-8 conversion
            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

            // Convert wstring to UTF-8 string
            std::string utf8Str = converter.to_bytes(wStr);

            // Access the underlying char array
            const char* charStr = utf8Str.c_str();
            return std::string(charStr);
        }

        bool endsWith(std::string const& fullString, std::string const& ending) {
            if (fullString.length() >= ending.length()) {
                return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
            }
            else {
                return false;
            }
        }

        void setPath(const fs::path& path) {
            if (fs::exists(path) && fs::is_directory(path)) {
                currentPath = path;
            }
        }

        void listDirectoryContents() {
            auto windowSize = ImGui::GetWindowSize();
            int numColumn = 10;

            auto itemSize = ImVec2(windowSize.x/numColumn-20, windowSize.x / numColumn-20);

            std::vector<fs::directory_entry> entries;

            for (const auto& entry : fs::directory_iterator(currentPath)) {
                entries.push_back(entry);
            }

            // Custom sort function to ensure folders come first
            std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
                return a.is_directory() > b.is_directory();
                });

            for (const auto& entry : entries) {
                std::string entryName = entry.path().filename().string();

                ImGui::Columns(numColumn, nullptr, false);
                ImGui::BeginGroup();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(-5, 0)); // Set padding to zero

                if (entry.is_directory()) {
                    // Load your folder icon texture here
                    ImGui::ImageButton((void*)m_folder->id(), itemSize);
                    if (ImGui::IsItemClicked()) {
                        currentPath = entry.path();
                    }
                } else {
                    // Load your file icon texture here
                    ImGui::ImageButton((void*)m_file->id(), itemSize);
                    if (ImGui::IsItemHovered() &&  ImGui::IsMouseDoubleClicked(0)) {
                        auto path = windowsToString(entry.path().c_str());
                            Prisma::SceneLoader sceneLoader;
                            auto scene = sceneLoader.loadScene(path, { true });
                            currentGlobalScene->root->addChild(scene->root, false);
                            currentGlobalScene->meshes.insert(currentGlobalScene->meshes.end(), scene->meshes.begin(), scene->meshes.end());
                            currentGlobalScene->omniLights.insert(currentGlobalScene->omniLights.end(), scene->omniLights.begin(), scene->omniLights.end());
                            currentGlobalScene->dirLights.insert(currentGlobalScene->dirLights.end(), scene->dirLights.begin(), scene->dirLights.end());

                            updateSizes = true;
                            skipUpdate = true;
                        
                    }
                }
                ImGui::PopStyleVar();  // Restore the previous padding
                ImGui::PopStyleColor();

                bool selected = false;

                auto textSize = ImGui::CalcTextSize(entryName.c_str());

                m_fontSize.x = textSize.x;

                ImGui::Text(entryName.c_str(), selected, 0, m_fontSize);
                ImGui::EndGroup();

                ImGui::NextColumn();
            }
        }
    public:
        FileBrowser() : currentPath(fs::current_path().parent_path().parent_path().parent_path()) {
            m_folder=std::make_shared<Prisma::Texture>();
            m_folder->loadTexture("../../../GUI/icons/folder.png",false,false,false);

            m_file=std::make_shared<Prisma::Texture>();
            m_file->loadTexture("../../../GUI/icons/file.png",false,false,false);

            m_back = std::make_shared<Prisma::Texture>();
            m_back->loadTexture("../../../GUI/icons/arrow.png", false, false, false);
            m_iconSize = ImVec2(24, 24);
            m_fontSize = m_iconSize;
        }

        void show(unsigned int width, unsigned int height, float offset, float scale, float translation) {
            ImGui::SetNextWindowPos(ImVec2(0, scale*height));
            ImGui::SetNextWindowSize(ImVec2(width,height - scale * height));

            ImGui::Begin("File Browser", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            if (ImGui::ImageButton((void*)m_back->id(), m_iconSize) && currentPath.has_parent_path()) {
                currentPath = currentPath.parent_path();
            }

            listDirectoryContents();

            ImGui::End();
        }
    };
}
