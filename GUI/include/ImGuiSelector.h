#pragma once
#include "bullet/btBulletDynamicsCommon.h"
#include "../../Engine/include/SceneObjects/Mesh.h"
#include "../../Engine/include/Physics/Physics.h"
#include "glm/glm.hpp"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../../Engine/include/Helpers/PrismaMath.h"

namespace Prisma {
    class ImGuiSelector {
    public:
        struct RayCastResult{
            Prisma::Mesh* other;
        };

        struct ScreenData{
            unsigned int width;
            unsigned int height;
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
        };

        ImGuiSelector();

        // Performs raycasting on the world and returns the point of collision
        std::shared_ptr<RayCastResult> raycastWorld(const glm::vec3 start, glm::vec3 end);

        // Ray casting from mouse coordinates
        std::pair<glm::vec3, glm::vec3> castRayFromMouse(double mouseX, double mouseY);

        std::shared_ptr<ScreenData> screenData(){
            return m_screenData;
        }

    private:
        std::shared_ptr<ScreenData> m_screenData;
    };
}
