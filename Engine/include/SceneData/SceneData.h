#pragma once
#include "../SceneObjects/Node.h"
#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/AnimatedMesh.h"
#include "../SceneObjects/Light.h"
#include "../SceneObjects/Camera.h"
#include <vector>
#include <string>
#include "../SceneObjects/Sprite.h"

namespace Prisma {
template <typename T>
class Light;

namespace LightType {
struct LightDir;
struct LightOmni;
struct LightArea;
}

struct SceneHandler {
    std::function<void()> onBeginRender = []() {
    };
    std::function<void(std::pair<std::string, int>)> onLoading = [](std::pair<std::string, int> node) {
    };
    std::function<void()> onEndRender = []() {
    };
    std::function<void()> onDestroy = []() {
    };
};

struct UserData {
    virtual void start() = 0;
    virtual void update() = 0;
    virtual void finish() = 0;
    virtual std::shared_ptr<CallbackHandler> callbacks() = 0;
};

struct Scene {
    std::shared_ptr<Node> root;
    std::shared_ptr<Camera> camera;
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<AnimatedMesh>> animateMeshes;
    std::vector<std::shared_ptr<Light<LightType::LightDir>>> dirLights;
    std::vector<std::shared_ptr<Light<LightType::LightOmni>>> omniLights;
    std::vector<std::shared_ptr<Light<LightType::LightArea>>> areaLights;
    std::vector<std::shared_ptr<Sprite>> sprites;
    std::string name;
};
}