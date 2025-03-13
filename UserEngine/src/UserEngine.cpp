#include "../include/UserEngine.h"
#include "../../Engine/include/Pipelines/PipelineSkybox.h"
#include "../../Engine/include/Components/PhysicsMeshComponent.h"


#include <memory>
#include <string>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <glm/gtc/random.hpp>
#include "../../Engine/include/SceneData/Animation.h"
#include <Windows.h>

void UserEngine::start()
{
    float lastX = 0.0f;  // Stores the last x position of the mouse
    float lastY = 0.0f;  // Stores the last y position of the mouse
    bool firstMouse = true; // To ensure the initial mouse position is captured

	float yaw = -90;
	float pitch = 0;

    auto callback = std::make_shared<Prisma::CallbackHandler>();
    m_camera = std::make_shared<Prisma::Camera>();
    Prisma::Engine::getInstance().mainCamera(m_camera);

    m_camera->center(m_camera->position() + m_camera->front());

    callback->keyboard = [&](int button, int action, double x, double y)
        {

            float velocity = 0.1f;

            float rotationAngle = glm::radians(5.0f); // Rotation step in degrees

            switch (button)
            {
            case 'W':
                m_position += m_front * velocity;
                break;
            case 'A':
                m_position -= glm::normalize(glm::cross(m_front, m_up)) * velocity;
                break;
            case 'S':
                m_position -= m_front * velocity;
                break;
            case 'D':
                m_position += glm::normalize(glm::cross(m_front, m_up)) * velocity;
                break;
            case 'Z': // Rotate the front vector
            {
                glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, m_up);
                m_front = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(m_front, 1.0f)));
                break;
            }
            case 'X': // Rotate in the opposite direction
            {
                glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), -rotationAngle, m_up);
                m_front = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(m_front, 1.0f)));
                break;
            }
            }
        };

    // Mouse callback to handle camera rotation based on mouse movement
    callback->mouse = [&](int x, int y)
        {
            float xpos = x;
            float ypos = y;

            if (m_firstMouse)
            {
                m_lastX = xpos;
                m_lastY = ypos;
                m_firstMouse = false;
            }

            float xoffset = xpos - m_lastX;
            float yoffset = m_lastY - ypos; // reversed since y-coordinates go from bottom to top
            m_lastX = xpos;
            m_lastY = ypos;

            float sensitivity = 0.1f; // change this value to your liking
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            m_yaw += xoffset;
            m_pitch += yoffset;

            // make sure that when pitch is out of bounds, screen doesn't get flipped
            if (m_pitch > 89.0f)
                m_pitch = 89.0f;
            if (m_pitch < -89.0f)
                m_pitch = -89.0f;

            glm::vec3 front;
            front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            front.y = sin(glm::radians(m_pitch));
            front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
            m_front = normalize(front);
        };
	Prisma::Engine::getInstance().setCallback(callback);
	Prisma::Engine::getInstance().getScene("../../../Resources/DefaultScene/default.prisma", {
		                                       true, [&](std::shared_ptr<Prisma::Scene> scene)
		                                       {
			                                       //m_player = std::make_shared<PlayerController>(scene);
												   //m_player->scene(scene);
												
													
		                                       }
	                                       });
}

void UserEngine::update()
{
	//m_player->update();
    m_camera->position(m_position);
    m_camera->center(m_position + m_front);
    m_camera->up(m_up);
    m_camera->front(m_front);
    m_camera->right(glm::normalize(glm::cross(m_front, m_up)));
}

void UserEngine::finish()
{
	//
}

std::shared_ptr<Prisma::CallbackHandler> UserEngine::callbacks()
{
	//return m_player->callback();
    return std::shared_ptr<Prisma::CallbackHandler>();
}
