#pragma once

#include "SceneObjects/Mesh.h"
#include "GlobalData/GlobalData.h"
#include "Helpers/PrismaMath.h"
#include "glm/gtx/matrix_decompose.hpp"
#include <functional>
#include "NodeViewer.h"

namespace Prisma {
class MeshInfo {
public:
        void showSelected(const NodeViewer::NodeData& meshData);

        MeshInfo();
};
}