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
	m_root = Prisma::Engine::getInstance().getScene("../../../Resources/DefaultScene/default.gltf", {true});
	Prisma::Texture texture;
	texture.loadEquirectangular("../../../Resources/Skybox/cloudy.hdr");
	texture.data({4096, 4096, 3});
	Prisma::PipelineSkybox::getInstance().texture(texture, true);

	Prisma::NodeHelper nodeHelper;

	nodeHelper.nodeIterator(m_root->root, [](auto mesh, auto parent)
	{
	});

	m_player = std::make_shared<PlayerController>(m_root);
}

void UserEngine::update()
{
	m_player->update();
}

void UserEngine::finish()
{
}

std::shared_ptr<Prisma::CallbackHandler> UserEngine::callbacks()
{
	return m_player->callback();
}
