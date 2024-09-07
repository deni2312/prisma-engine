#pragma once

#include "../../Engine/include/SceneObjects/Mesh.h"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "imgui.h"
#include "../../Engine/include/Helpers/PrismaMath.h"
#include "glm/gtx/matrix_decompose.hpp"
#include <functional>
#include "ImGuizmo.h"
#include "NodeViewer.h"

namespace Prisma {
    class MeshInfo {
    public:

        void showSelected(const Prisma::NodeViewer::NodeData& meshData);

        MeshInfo();

    };
}

