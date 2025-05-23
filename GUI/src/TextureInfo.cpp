#include "../include/TextureInfo.h"

#include <algorithm>

#include "GlobalData/GlobalData.h"
#include "../include/ImGuiHelper.h"
#include "ThirdParty/imgui/imgui.h"

Prisma::GUI::TextureInfo::TextureInfo() : m_index{-1} {
}

void Prisma::GUI::TextureInfo::showTextures() {
    constexpr int numColumns = 5;
    int scale = 10;

    auto data = [&](int i) {
        if (ImGui::BeginTable("TextureTable", numColumns)) {
            for (int j = 0; j < numColumns; j++) {
                if (i * numColumns + j < GlobalData::getInstance().globalTextures().size()) {
                    ImGui::TableNextColumn(); // Move to the next column

                    auto texture = GlobalData::getInstance().globalTextures()[i * numColumns + j];

                    // Display the image
                    ImGui::PushID(i * numColumns + j);
                    if (ImGui::ImageButton(
                        texture.texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE),
                        ImVec2(100, 100))) {
                        m_index = i * numColumns + j;
                        m_textureTab = true;
                    }
                    ImGui::PopID();
                    // Get last part of the file path
                    auto getLast = [](std::string s) {
                        size_t found = s.find_last_of('/');
                        return found != std::string::npos ? s.substr(found + 1) : s;
                    };

                    // Display the text under the image
                    ImGui::TextWrapped("%s", getLast(texture.name).c_str());
                } else {
                    break;
                }
            }
            ImGui::EndTable();
        }
        ImGui::NewLine();
    };
    auto size = GlobalData::getInstance().globalTextures().size();
    if (size % numColumns > 0) {
        size = size / numColumns + 1;
    } else {
        size = size / numColumns;
    }
    ImGuiHelper::getInstance().clipVertical(size, data);

    if (m_index > -1) {
        ImGui::Begin("Texture Preview", &m_textureTab, ImGuiWindowFlags_AlwaysAutoResize);
        auto texture = GlobalData::getInstance().globalTextures()[m_index];
        ImGui::InputFloat("Scaling", &m_scale);
        if (m_scale == 0) {
            m_scale = 1;
        }
        ImGui::Text("Selected Texture:");
        ImGui::Image(texture.texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE),
                     ImVec2(texture.size.x * m_scale, texture.size.y * m_scale));
        ImGui::TextWrapped("%s", texture.name.c_str());
        ImGui::End();
        if (!m_textureTab) {
            m_index = -1;
        }
    }
}

bool Prisma::GUI::TextureInfo::textureTab() {
    return m_textureTab;
}