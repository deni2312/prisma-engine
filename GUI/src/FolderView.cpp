#include "../include/FolderView.h"
#include "../include/TextureInfo.h"
#include "../include/ImGuiHelper.h"
#include "../../Engine/include/Handlers/LoadingHandler.h"
#include "../../Engine/include/Helpers/StringHelper.h"

std::string Prisma::FileBrowser::windowsToString(std::wstring wStr)
{
	// Create a codecvt facet for UTF-8 conversion
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

	// Convert wstring to UTF-8 string
	std::string utf8Str = converter.to_bytes(wStr);

	// Access the underlying char array
	const char* charStr = utf8Str.c_str();
	return std::string(charStr);
}

void Prisma::FileBrowser::setPath(const fs::path& path)
{
	if (exists(path) && is_directory(path))
	{
		m_currentPath = path;
	}
}

void Prisma::FileBrowser::listDirectoryContents()
{
	auto windowSize = ImGui::GetWindowSize();
	int numColumn = 10;

	auto itemSize = ImVec2(100, 100);
	bool isDirectory = false;
	fs::directory_entry entryPath;
	auto data = [&](int i) {
		if (ImGui::BeginTable("FolderTable", numColumn)) {
			for (int j = 0; j < numColumn; j++) {
				if (i * numColumn + j < m_entries.size()) {
					try
					{
						ImGui::TableNextColumn(); // Move to the next column
						auto entry = m_entries[i * numColumn+j];
						std::string entryName = entry.path().filename().string();


						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));

						if (entry.is_directory())
						{
							// Load your folder icon texture here
							ImGui::ImageButton((void*)m_folder->id(), itemSize);
							if (ImGui::IsItemClicked())
							{
								isDirectory = true;
								entryPath = entry;
							}
						}
						else
						{
							// Load your file icon texture here
							ImGui::ImageButton((void*)m_file->id(), itemSize);
							if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
							{
								auto path = windowsToString(entry.path().c_str());

								if (Prisma::StringHelper::getInstance().endsWith(path, ".prisma"))
								{
									if (Prisma::GlobalData::getInstance().currentGlobalScene()->root)
									{
										Prisma::LoadingHandler::getInstance().load(path, { true, nullptr, true });
									}
									else
									{
										Prisma::LoadingHandler::getInstance().load(path, { true, nullptr, false });
									}
								}
								else
								{
									Prisma::SceneLoader loader;
									auto scene = loader.loadScene(path, { true });
									if (Prisma::GlobalData::getInstance().currentGlobalScene()->root)
									{
										Prisma::GlobalData::getInstance().currentGlobalScene()->root->addChild(scene->root);
									}
									else
									{
										Prisma::GlobalData::getInstance().currentGlobalScene(scene);
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

					}
					catch (std::exception& e)
					{
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
	}
	else {
		size = size / numColumn;
	}
	Prisma::ImGuiHelper::getInstance().clipVertical(size, data);


	if (isDirectory)
	{
		m_currentPath = entryPath.path();
		m_entries.clear();

		for (const auto& entry : fs::directory_iterator(m_currentPath))
		{
			m_entries.push_back(entry);
		}

		// Custom sort function to ensure folders come first
		std::sort(m_entries.begin(), m_entries.end(),
		          [](const fs::directory_entry& a, const fs::directory_entry& b)
		          {
			          return a.is_directory() > b.is_directory();
		          });
	}
}

Prisma::FileBrowser::FileBrowser() : m_currentPath(fs::current_path().parent_path().parent_path().parent_path())
{
	m_folder = std::make_shared<Texture>();
	m_folder->loadTexture({"../../../GUI/icons/folder.png", false, false, false});

	m_file = std::make_shared<Texture>();
	m_file->loadTexture({"../../../GUI/icons/file.png", false, false, false});

	m_back = std::make_shared<Texture>();
	m_back->loadTexture({"../../../GUI/icons/arrow.png", false, false, false});
	m_iconSize = ImVec2(24, 24);
	m_fontSize = m_iconSize;

	for (const auto& entry : fs::directory_iterator(m_currentPath))
	{
		m_entries.push_back(entry);
	}

	// Custom sort function to ensure folders come first
	std::sort(m_entries.begin(), m_entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b)
	{
		return a.is_directory() > b.is_directory();
	});
}

void Prisma::FileBrowser::show(unsigned int width, unsigned int height, float offset, float scale, float translation)
{
	ImGui::SetNextWindowPos(ImVec2(0, scale * height + offset));
	ImGui::SetNextWindowSize(ImVec2(width, height - (scale * height + offset)));

	ImGui::Begin("File Browser", nullptr,
	             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);


	ImGui::BeginTabBar("FOLDER VIEW");


	if (ImGui::BeginTabItem("Textures"))
	{
		TextureInfo::getInstance().showTextures();
		ImGui::EndTabItem();
	}


	if (ImGui::BeginTabItem("Logs"))
	{
		m_logger.render();
		ImGui::EndTabItem();
	}


	if (ImGui::BeginTabItem("Folders##1"))
	{
		if (ImGui::ImageButton((void*)m_back->id(), m_iconSize) && m_currentPath.has_parent_path())
		{
			m_currentPath = m_currentPath.parent_path();

			m_entries.clear();

			for (const auto& entry : fs::directory_iterator(m_currentPath))
			{
				m_entries.push_back(entry);
			}

			// Custom sort function to ensure folders come first
			std::sort(m_entries.begin(), m_entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b)
			{
				return a.is_directory() > b.is_directory();
			});
		}
		listDirectoryContents();
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
	ImGui::End();
}
