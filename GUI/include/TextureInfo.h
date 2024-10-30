#pragma once

#include "imgui.h"
#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include <string>
#include "../../Engine/include/GlobalData/InstanceData.h"

namespace Prisma
{
	class TextureInfo : public InstanceData<TextureInfo>
	{
	public:
		TextureInfo();

		void showTextures();

		void add(std::pair<unsigned int, std::string> id);

	private:
		std::vector<std::pair<unsigned int, std::string>> m_textures;
	};
}
