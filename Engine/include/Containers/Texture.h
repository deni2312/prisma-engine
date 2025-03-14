#pragma once
#include <string>
#include <vector>

#include "../GlobalData/Platform.h"

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/Texture.h"

namespace Diligent
{
	struct IShaderResourceBinding;
}

namespace Prisma
{
	class Texture
	{
	public:
		struct TextureData
		{
			int width;
			int height;
			int nrComponents;
			unsigned char* dataContent = nullptr;
			bool deleteStbi = true;
		};

		struct Parameters
		{
			std::string texture;
			bool srgb = false;
			bool local = true;
		};

		bool loadTexture(const Parameters& parameters);
		uint64_t id() const;
		void id(uint64_t id);
		unsigned int rawId() const;
		void rawId(unsigned int rawId);
		std::string name() const;
		void name(std::string name);
		bool loadCubemap(std::vector<std::string> faces, bool srgb = false);
		bool loadEquirectangular(std::string texture);
		TextureData data() const;
		void data(TextureData data);
		void freeData();

		Diligent::RefCntAutoPtr<Diligent::ITexture> texture();

		const Parameters parameters() const;

		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> shader();

	private:
		uint64_t m_id = 0;
		unsigned int m_rawId = 0;
		TextureData m_data;
		Parameters m_parameters;
		std::string m_name;
		Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_shader;
	};
}
