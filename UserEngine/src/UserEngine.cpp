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
	auto callback = std::make_shared<Prisma::CallbackHandler>();
	m_camera = std::make_shared<Prisma::Camera>();
	Prisma::Engine::getInstance().mainCamera(m_camera);
	m_camera->center(m_camera->position() + m_camera->front());
	callback->keyboard = [&](int key,Prisma::CallbackHandler::ACTION action)
	{
			auto camera = m_camera;
			auto position = camera->position();
			auto front = camera->front();
			auto up = camera->up();
			float velocity = 0.1;

			switch (key)
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
