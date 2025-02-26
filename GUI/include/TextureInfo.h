#pragma once

#include "imgui.h"
#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include <string>
#include "../../Engine/include/GlobalData/InstanceData.h"
#include "../../Engine/include/Containers/Texture.h"

namespace Prisma
{
	class TextureInfo : public InstanceData<TextureInfo>
	{
	public:
		TextureInfo();

		void showTextures();
	private:

		int m_index;
	};
}
