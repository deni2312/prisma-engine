#include "../include/TextureInfo.h"

std::shared_ptr<Prisma::TextureInfo> Prisma::TextureInfo::instance = nullptr;

Prisma::TextureInfo::TextureInfo() {

}

void Prisma::TextureInfo::showTextures() {

    const int numColumns = 5;
    ImGui::Columns(numColumns, nullptr, false);

    // Display the images in a grid layout
    for (auto id : m_textures) {
        ImGui::Image((void*)(intptr_t)id.first, ImVec2(100, 100));
        auto getLast = [](std::string s) {
            size_t found = s.find_last_of('/');
            return found != std::string::npos ? s.substr(found + 1) : s;
        };
        ImGui::Text(getLast(id.second).c_str());
        ImGui::NextColumn();
    }

    // End the columns
    ImGui::Columns(1);
}

void Prisma::TextureInfo::add(std::pair<unsigned int, std::string> id) {
    m_textures.push_back(id);
}

Prisma::TextureInfo& Prisma::TextureInfo::getInstance()
{
    if (!instance) {
        instance = std::make_shared<TextureInfo>();
    }
    return *instance;
}
