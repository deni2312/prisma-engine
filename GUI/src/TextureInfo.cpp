#include "../include/TextureInfo.h"

Prisma::TextureInfo::TextureInfo()
{
}

void Prisma::TextureInfo::showTextures()
{
	constexpr int numColumns = 5;
	ImGui::Columns(numColumns, nullptr, false);
	int scale = 10;
	// Display the images in a grid layout
	for (auto texture : m_textures)
	{
		int width = texture.data().width;
		int height = texture.data().height;
		width = width / scale;
		height = height / scale;

		ImGui::Image((void*)static_cast<intptr_t>(texture.id()), ImVec2(width, height));
		auto getLast = [](std::string s)
		{
			size_t found = s.find_last_of('/');
			return found != std::string::npos ? s.substr(found + 1) : s;
		};
		ImGui::Text(getLast(texture.name()).c_str());
		ImGui::NextColumn();
	}

	// End the columns
	ImGui::Columns(1);
}

void Prisma::TextureInfo::add(Prisma::Texture id)
{
	m_textures.push_back(id);
}
