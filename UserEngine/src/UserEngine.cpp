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

void UserEngine::start()
{
	Prisma::Engine::getInstance().setCallback(std::make_shared<Prisma::CallbackHandler>());
	Prisma::Engine::getInstance().getScene("../../../Resources/DefaultScene/default.prisma", {
		                                       true, [&](auto scene)
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
