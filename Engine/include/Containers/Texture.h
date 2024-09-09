#pragma once
#include <string>
#include <vector>

namespace Prisma {

	class Texture {
	public:
		struct TextureData {
			int width;
			int height;
			int nrComponents;
		};
		bool loadTexture(std::string texture, bool srgb = false,bool resident=true,bool noRepeat=true,bool mantainData=false);
		uint64_t id() const;
		void id(uint64_t id);
		std::string name() const;
		void name(std::string name);
		bool loadCubemap(std::vector<std::string> faces,bool srgb = false);
		bool loadEquirectangular(std::string texture);
		TextureData data() const;
		void data(TextureData data);
		unsigned char* dataContent();
		void freeData();
	private:
		uint64_t m_id=0;
		std::string m_name="";
		TextureData m_data;
		unsigned char* m_dataContent = nullptr;
	};
}