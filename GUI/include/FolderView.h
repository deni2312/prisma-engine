#include <filesystem>
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include "../../Engine/include/Containers/Texture.h"
#include "imgui.h"
#include "../../Engine/include/SceneData/SceneLoader.h"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../../Engine/include/GlobalData/CacheScene.h"
#include "../../Engine/include/SceneData/MeshIndirect.h"

namespace Prisma
{
	namespace fs = std::filesystem;

	class FileBrowser
	{
		ImVec2 m_fontSize;
		ImVec2 m_iconSize;

		fs::path currentPath;
		std::shared_ptr<Texture> m_folder;
		std::shared_ptr<Texture> m_file;
		std::shared_ptr<Texture> m_back;
		std::vector<fs::directory_entry> m_entries;

		std::string windowsToString(std::wstring wStr);

		bool endsWith(const std::string& fullString, const std::string& ending);

		void setPath(const fs::path& path);

		void listDirectoryContents();

	public:
		FileBrowser();

		void show(unsigned int width, unsigned int height, float offset, float scale, float translation);
	};
}
