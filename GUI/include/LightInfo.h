#pragma once

#include "../../Engine/include/SceneObjects/Light.h"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "imgui.h"
#include "../../Engine/include/Helpers/PrismaMath.h"
#include "glm/gtx/matrix_decompose.hpp"
#include <functional>
#include "ImGuizmo.h"
#include "MeshInfo.h"

namespace Prisma {
    class LightInfo {
    public:

        void showSelectedDir(Prisma::Light<Prisma::LightType::LightDir>* lightData,const Prisma::MeshInfo::MeshData& meshData);
        void showSelectedOmni(Prisma::Light<Prisma::LightType::LightOmni>* lightData,const Prisma::MeshInfo::MeshData& meshData);

    private:
        glm::vec3 m_translation;

        glm::mat4 m_currentModelDir;

        glm::mat4 m_currentModelOmni;

        ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::ROTATE;
        ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;

        glm::vec3 directionToEulerAngles(const glm::vec3& direction);
    };
}

