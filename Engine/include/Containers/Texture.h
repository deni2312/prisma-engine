#pragma once
#include <string>
#include <vector>

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
			bool resident = true;
			bool noRepeat = true;
			bool mantainData = false;
			bool anisotropic = true;
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

		const Parameters parameters() const;

	private:
		uint64_t m_id = 0;
		unsigned int m_rawId = 0;
		TextureData m_data;
		Parameters m_parameters;
		std::string m_name;
	};
}
