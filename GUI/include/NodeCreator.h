#pragma once

#include <memory>
#include "SceneObjects/Mesh.h"
#include "GlobalData/GlobalData.h"


namespace Prisma::GUI {
class NodeCreator {
public:
    static std::shared_ptr<Node> createCube();

    static std::shared_ptr<Node> createSphere(int subDivisions);

    static std::shared_ptr<Node> createOmnidirectional();

    static std::shared_ptr<Node> createDirectional();

    static std::shared_ptr<Node> createArea();

    static std::shared_ptr<Node> createNode();

private:
    static std::shared_ptr<Node> createMesh(std::shared_ptr<Mesh::VerticesData> verticesData,
                                            const std::string& name);

    static std::shared_ptr<MaterialComponent> getEmptyMaterial();
};
}