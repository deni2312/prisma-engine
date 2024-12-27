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
	Prisma::Engine::getInstance().getScene("../../../Resources/DefaultScene/default.prisma", {
		                                       true, [&](auto scene)
		                                       {
			                                       Prisma::Texture texture;
			                                       texture.loadEquirectangular(
				                                       "../../../Resources/Skybox/cloudy.hdr");
			                                       texture.data({4096, 4096, 3});
			                                       Prisma::PipelineSkybox::getInstance().
				                                       texture(texture, true);
			                                       //m_player = std::make_shared<PlayerController>(scene);

			                                       //m_player->scene(scene);

												   Prisma::NodeHelper nodeHelper;

												   auto transparentMesh = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(scene->root, "Cube.001"));
												   auto transparentMesh1 = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(scene->root, "Cube.003"));
												   auto transparentMesh11 = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(scene->root, "Cube.004"));
												   auto transparentMesh16 = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(scene->root, "Cube.005"));
												   if (transparentMesh && transparentMesh1 && transparentMesh11 && transparentMesh16)
												   {
													   transparentMesh->material()->transparent(true);
													   transparentMesh1->material()->transparent(true);
													   transparentMesh11->material()->transparent(true);
													   transparentMesh16->material()->transparent(true);
												   }

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
