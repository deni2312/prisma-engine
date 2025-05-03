#include <filesystem>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

#include "ImGuiLog.h"
#include "Containers/Texture.h"
#include "glm/vec2.hpp"

namespace Prisma::GUI {
namespace fs = std::filesystem;

class FileBrowser {
    glm::vec2 m_fontSize;
    glm::vec2 m_iconSize;

    fs::path m_currentPath;
    std::shared_ptr<Texture> m_folder;
    std::shared_ptr<Texture> m_file;
    std::shared_ptr<Texture> m_back;
    std::vector<fs::directory_entry> m_entries;
    ImGuiLog m_logger;

    std::string windowsToString(std::wstring wStr);

    void setPath(const fs::path& path);

    void listDirectoryContents();

    void addEntries();

public:
    FileBrowser();

    void show(unsigned int width, unsigned int height, float offset, float scale, float translation);
};
}