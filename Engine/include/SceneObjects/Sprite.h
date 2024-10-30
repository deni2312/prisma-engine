#pragma once
#include <vector>
#include <memory>
#include "Node.h"
#include "../Containers/Texture.h"
#include "../Helpers/Shader.h"
#include "../Containers/SSBO.h"

namespace Prisma
{
	class Sprite : public Node
	{
	public:
		Sprite();
		void loadSprites(std::vector<std::shared_ptr<Texture>> textures);
		void numSprites(unsigned int numSprites);
		void size(glm::vec2 size);
		void render();

	private:
		struct SpriteData
		{
			uint64_t sprite;
			glm::vec2 padding;
		};

		std::vector<SpriteData> m_spritesData;
		std::shared_ptr<SSBO> m_ssbo;
		std::shared_ptr<SSBO> m_ssboTextures;
		std::shared_ptr<SSBO> m_ssboIds;
		unsigned int m_spritePos;
		unsigned int m_modelPos;
		unsigned int m_numSprites = 0;
		unsigned int m_sizePos;
		glm::vec2 m_size = glm::vec2(1.0f, 1.0f);
		std::shared_ptr<Shader> m_spriteShader;
	};
}
