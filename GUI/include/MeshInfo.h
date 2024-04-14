#pragma once

#include "../../Engine/include/SceneObjects/Mesh.h"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "imgui.h"
#include "../../Engine/include/Helpers/PrismaMath.h"
#include "glm/gtx/matrix_decompose.hpp"
#include <functional>
#include "ImGuizmo.h"

namespace Prisma {
    class MeshInfo {
    public:

        struct MeshData{
            Prisma::Mesh *mesh;
            std::shared_ptr<Prisma::Camera> camera;
            glm::mat4 projection;
            float translate;
            float width;
            float height;
            float scale;
            float initOffset;
        };

        void showSelected(MeshData meshData);
        void showComponents(MeshData meshData);
        void varsDispatcher(Prisma::Component::ComponentType types);

        MeshInfo();

        std::function<void(glm::mat4&)> updateMesh();

    private:

        glm::vec3 m_scale;
        glm::vec3 m_rotation;
        glm::vec3 m_translation;

        glm::mat4 m_currentModel;

        std::shared_ptr<Prisma::Texture> m_rotateTexture;
        std::shared_ptr<Prisma::Texture> m_translateTexture;
        std::shared_ptr<Prisma::Texture> m_scaleTexture;

        ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;

        void drawGizmo(Prisma::MeshInfo::MeshData meshData);
    };
}

