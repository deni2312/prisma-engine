#include "../include/TextureInfo.h"

#include <algorithm>

#include "GL/glew.h"
#include "../../Engine/include/GlobalData/GlobalData.h"

Prisma::TextureInfo::TextureInfo()
{
}

void Prisma::TextureInfo::showTextures()
{
	constexpr int numColumns = 5;
	ImGui::Columns(numColumns, nullptr, false);
	int scale = 10;

	// Display the images in a grid layout
	for (auto texture : Prisma::GlobalData::getInstance().globalTextures())
	{
		ImGui::Image((void*)static_cast<intptr_t>(texture.first), ImVec2(100, 100));
		auto getLast = [](std::string s)
		{
			size_t found = s.find_last_of('/');
			return found != std::string::npos ? s.substr(found + 1) : s;
		};
		ImGui::Text(getLast(texture.second).c_str());
		ImGui::NextColumn();
	}

	// End the columns
	ImGui::Columns(1);
}
