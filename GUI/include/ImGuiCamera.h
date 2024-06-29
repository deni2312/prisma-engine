#pragma once
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Helpers/Settings.h"
#include <memory>

namespace Prisma {

	class ImGuiCamera {
	public:
        struct CameraConstarints{
            float minX;
            float minY;
            float maxX;
            float maxY;
            std::function<void(glm::mat4&)> model;
			bool isOver;
			float scale;
        };

		ImGuiCamera();
		void updateCamera(std::shared_ptr<Prisma::Camera> camera);
		void keyboardUpdate(void* windowData);
		void mouseCallback();
        void mouseButtonCallback();
        void velocity(float velocity);
		void currentSelect(Node* currentSelect);
        Node* currentSelect();

        void constraints(CameraConstarints constraints);

		std::shared_ptr<Prisma::CallbackHandler> callback();
	private:

		glm::vec3 m_position = glm::vec3(0.0f);
		glm::vec3 m_front = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);

		float m_yaw = -90.0f;
		float m_pitch = 0.0f;
		float m_velocity = 1.0;
		bool firstMouse = true;
		float m_lastX;
		float m_lastY;
		bool m_lock = false;

		bool m_showMouse = false;
		bool m_pressed = false;
		bool m_save = false;

		std::shared_ptr<Prisma::CallbackHandler> m_callback;

        Node* m_currentSelect;
        CameraConstarints m_constraints;
	};
}