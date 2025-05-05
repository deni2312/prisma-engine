#pragma once
#include "../GlobalData/InstanceData.h"
#include <string>
#include "../SceneData/SceneLoader.h"

namespace Prisma {
class LoadingHandler : public InstanceData<LoadingHandler> {
public:
    void load(std::string scene, SceneLoader::SceneParameters sceneParameters);
    void update(std::shared_ptr<Camera> camera, std::function<void(std::pair<std::string, int>)> loading);

private:
    SceneLoader::SceneParameters m_sceneParameters;
    SceneLoader m_loader;
    bool m_hasLoad = false;
};
}