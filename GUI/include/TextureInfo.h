#pragma once
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

		bool textureTab();
	private:

		int m_index;

		bool m_textureTab = false;

		float m_scale = 1;
	};
}
