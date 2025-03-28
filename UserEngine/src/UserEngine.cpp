#include "../include/UserEngine.h"
#include "../../Engine/include/Pipelines/PipelineSkybox.h"
#include "../../Engine/include/Components/PhysicsMeshComponent.h"
#include "../../Engine/include/SceneData/SceneLoader.h"

#include <memory>

void UserEngine::start()
{
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
	//return m_player->callback();
    return std::shared_ptr<Prisma::CallbackHandler>();
}
