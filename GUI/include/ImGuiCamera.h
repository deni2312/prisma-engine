#pragma once
#include "SceneObjects/Camera.h"
#include "Helpers/Settings.h"
#include <memory>

namespace Prisma {
class ImGuiCamera {
public:
        struct CameraConstarints {
                float minX;
                float minY;
                float maxX;
                float maxY;
                bool isOver;
                float scale;
                glm::mat4 model;
        };

        ImGuiCamera();
        void updateCamera(std::shared_ptr<Camera> camera);
        void keyboardUpdate(void* windowData);
        void mouseCallback();
        void mouseButtonCallback();
        void velocity(float velocity);
        void currentSelect(std::shared_ptr<Node> currentSelect);
        std::shared_ptr<Node> currentSelect();

        void constraints(CameraConstarints constraints);

        std::shared_ptr<CallbackHandler> callback();

        bool openPopup() const;

        void openPopup(bool openPopup);

private:
        glm::vec3 m_position = glm::vec3(0.0f);
        glm::vec3 m_front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);

        float m_yaw = -90.0f;
        float m_pitch = 0.0f;
        float m_velocity = 1.0;
        bool m_firstMouse = true;
        float m_lastX;
        float m_lastY;

        bool m_showMouse = false;
        bool m_pressed = false;
        bool m_save = false;
        bool m_right = false;

        bool m_openPopup = false;

        float m_totalVelocity = 0;

        std::shared_ptr<CallbackHandler> m_callback;

        std::shared_ptr<Node> m_currentSelect;
        CameraConstarints m_constraints;
};
}