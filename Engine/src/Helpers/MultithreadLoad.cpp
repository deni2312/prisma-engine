#include "../../include/Helpers/MultithreadLoad.h"
#include <future>
#include <thread>
#include <vector>
#include <string>
#include "../../include/Helpers/stb_image.h"

std::vector<Prisma::Texture::TextureData> Prisma::MultithreadLoad::loadTextures(std::vector<std::string> data, unsigned int numThreads)
{
    std::vector<std::vector<std::string>> dataParts;
    splitData(data, numThreads, dataParts);

    std::vector<std::future<std::vector<Prisma::Texture::TextureData>>> futures;

    for (unsigned int i = 0; i < numThreads; ++i)
    {
        futures.emplace_back(std::async(&MultithreadLoad::loadPart, this, std::ref(dataParts[i])));
    }

    std::vector<Prisma::Texture::TextureData> result;

    for (auto& future : futures)
    {
        auto threadResult = future.get();
        result.insert(result.end(), threadResult.begin(), threadResult.end());
    }

    return result;
}

void Prisma::MultithreadLoad::splitData(const std::vector<std::string>& data, unsigned int numThreads, std::vector<std::vector<std::string>>& dataParts)
{
    // Split the input data into parts for each thread
    unsigned int dataSize = data.size();
    unsigned int partSize = dataSize / numThreads;
    unsigned int remaining = dataSize % numThreads;

    unsigned int start = 0;
    unsigned int end = 0;

    for (unsigned int i = 0; i < numThreads; ++i)
    {
        end = start + partSize + (i < remaining ? 1 : 0);
        dataParts.emplace_back(data.begin() + start, data.begin() + end);
        start = end;
    }
}

std::vector<Prisma::Texture::TextureData> Prisma::MultithreadLoad::loadPart(std::vector<std::string> data)
{
    std::vector<Prisma::Texture::TextureData> textureLoaded;
    for (auto d : data)
    {
        Prisma::Texture::TextureData textureData;
        //textureData.data = stbi_load(d.c_str(), &textureData.width, &textureData.height, &textureData.nrComponents, 0);
        textureLoaded.push_back(textureData);
    }
    return textureLoaded;
}
