#pragma once
#include <memory>
#include <iostream>
#include "../SceneObjects/Node.h"

namespace Prisma {
class ScenePrinter {
public:
    void printScene(std::shared_ptr<Node> nodeRoot, int depth);
};
}