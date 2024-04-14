#pragma once
#include <vector>
#include "../Containers/Texture.h"

namespace Prisma {
	class MultithreadLoad {
	public:
		std::vector<Prisma::Texture::TextureData> loadTextures(std::vector<std::string> data,unsigned int numThreads);
	private:
		std::vector< Prisma::Texture::TextureData> loadPart(std::vector<std::string> data);
		void Prisma::MultithreadLoad::splitData(const std::vector<std::string>& data, unsigned int numThreads, std::vector<std::vector<std::string>>& dataParts);
	};
}