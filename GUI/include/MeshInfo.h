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

        std::function<void(glm::mat4&)> updateMesh();

    private:

        glm::vec3 m_scale;
        glm::vec3 m_rotation;
        glm::vec3 m_translation;

        glm::mat4 m_currentModel;

        ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;

        void drawGizmo(const Prisma::NodeViewer::NodeData& meshData);
    };
}

