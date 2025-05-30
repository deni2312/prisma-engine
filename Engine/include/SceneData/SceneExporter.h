#pragma once
#include <memory>

#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/Light.h"
#include "../GlobalData/InstanceData.h"
#include "assimp/Exporter.hpp"

namespace Prisma {
class Exporter : public InstanceData<Exporter> {
public:
    Exporter();

    void exportScene(const std::string& sceneName = "output.prisma");

    void importSceneAsync(const std::string& sceneName);

    std::shared_ptr<Node> importScene(const std::string& sceneName);

    bool hasFinish();

    std::shared_ptr<Node> newRootNode();

    std::mutex& mutexData();

    std::pair<std::string, int> status();
    void postLoad(std::shared_ptr<Node> node, bool loadCubemap = true);

private:
    std::string getFileName(const std::string& filePath);
    std::atomic_bool m_finish;
    std::shared_ptr<Node> m_newRootNode = nullptr;

    void countNodes(std::shared_ptr<Node> next, int& counter);
    void loadTexturesMultithreaded(std::vector<std::shared_ptr<Mesh>>& meshes,
                                   std::unordered_map<std::string, Texture>& texturesLoaded, int numThreads);
};
}