#pragma once
#include <vector>
#include <memory>
#include "Node.h"
#include "GlobalData/Platform.h"
#include "Containers/Texture.h"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"

namespace Diligent
{
	struct IPipelineResourceSignature;
	struct IPipelineState;
}

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
		std::vector<Diligent::IDeviceObject*> m_sprites;

		unsigned int m_spritePos;
		unsigned int m_modelPos;
		unsigned int m_numSprites = 0;
		unsigned int m_sizePos;
		glm::vec2 m_size = glm::vec2(1.0f, 1.0f);

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;
		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_models;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_spriteIds;
	};
}
