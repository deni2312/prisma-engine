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
            auto camera = m_camera;
            auto position = camera->position();
            auto front = camera->front();
            auto up = camera->up();
            float velocity = 0.1f;

            switch (button)
            {
            case 'W':
                position += front * velocity;
                break;
            case 'A':
                position -= glm::normalize(glm::cross(front, up)) * velocity;
                break;
            case 'S':
                position -= front * velocity;
                break;
            case 'D':
                position += glm::normalize(glm::cross(front, up)) * velocity;
                break;
            }

            camera->position(position);
            camera->center(position + front);
            camera->front(front);
            camera->up(up);
        };

    // Mouse callback to handle camera rotation based on mouse movement
    callback->mouse = [&](int x, int y)
        {
			float xpos = x;
			float ypos = y;

			if (firstMouse)
			{
				lastX = xpos;
				lastY = ypos;
				firstMouse = false;
			}

			float xoffset = xpos - lastX;
			float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
			lastX = xpos;
			lastY = ypos;

			float sensitivity = 0.1f; // change this value to your liking
			xoffset *= sensitivity;
			yoffset *= sensitivity;

			yaw += xoffset;
			pitch += yoffset;

			// make sure that when pitch is out of bounds, screen doesn't get flipped
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;

			glm::vec3 front;
			front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			front = normalize(front);
			m_camera->position(m_camera->position());
			m_camera->center(m_camera->position() + front);
			m_camera->up(m_camera->up());
			m_camera->front(front);
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
}

void UserEngine::finish()
{
	//
}

std::shared_ptr<Prisma::CallbackHandler> UserEngine::callbacks()
{
	return m_player->callback();
}
