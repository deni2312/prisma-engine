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

std::shared_ptr<Prisma::Animator> animator;
std::shared_ptr<Prisma::Animator> animator1;

UserEngine::UserEngine() : Prisma::Engine{}
{
	m_sceneNode = getScene("../../../Resources/Helmet/DamagedHelmet.gltf", {true});
	Prisma::Texture texture;
	texture.loadEquirectangular("../../../Resources/Skybox/equirectangular.hdr");
	texture.data({ 4096,4096,3 });
	Prisma::PipelineSkybox::getInstance().texture(texture,true);

    Prisma::NodeHelper nodeHelper;

    


    Prisma::Physics::getInstance().physicsWorld()->dynamicsWorld->setGravity(btVector3(0.0,-10.0,0.0));
    
}

bool UserEngine::update()
{

    return false;
}

void UserEngine::mouseCallback()
{

}
