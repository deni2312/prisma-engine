#include "../include/TextureInfo.h"

std::shared_ptr<Prisma::TextureInfo> Prisma::TextureInfo::instance = nullptr;

Prisma::TextureInfo::TextureInfo() {

}

void Prisma::TextureInfo::showTextures() {
    bool closed = true;
    if (ImGui::BeginPopupModal("Textures",&closed)) {
        if (!m_position) {
            ImGui::SetWindowPos(ImVec2(0, 0));
            m_position = true;
        }
        const int numColumns = 5;
        ImGui::Columns(numColumns, nullptr, false);

        // Display the images in a grid layout
        for (auto id : m_textures) {
            ImGui::Image((void*)(intptr_t)id.first, ImVec2(100, 100));
            ImGui::NextColumn();
        }

        // End the columns
        ImGui::Columns(1);
        if (!closed)
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void Prisma::TextureInfo::add(std::pair<unsigned int, glm::vec2> id) {
    m_textures.push_back(id);
}

Prisma::TextureInfo& Prisma::TextureInfo::getInstance()
{
    if (!instance) {
        instance = std::make_shared<TextureInfo>();
    }
    return *instance;
}
