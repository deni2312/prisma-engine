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
	m_sceneNode = getScene("../../../Resources/Landscape/landscape.gltf", {true});
	Prisma::Texture texture;
	texture.loadEquirectangular("../../../Resources/Skybox/equirectangular.hdr");
	texture.data({ 4096,4096,3 });
	Prisma::PipelineSkybox::getInstance().texture(texture,true);

    Prisma::NodeHelper nodeHelper;

    nodeHelper.nodeIterator(m_sceneNode->root, [](auto mesh, auto parent) {
        auto currentMesh = std::dynamic_pointer_cast<Prisma::Mesh>(mesh);
        if (currentMesh) {
            auto physicsComponent = std::make_shared<Prisma::PhysicsMeshComponent>();
            physicsComponent->collisionData({ Prisma::Physics::Collider::BOX_COLLIDER,0.0,btVector3(0.0,0.0,0.0),true });
            currentMesh->addComponent(physicsComponent);
        }
        });
    auto torusInstance = Prisma::Mesh::instantiate(std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(m_sceneNode->root, "Torus")->children()[0]));

    std::cout << torusInstance;

    auto animatedMesh = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(nodeHelper.find(m_sceneNode->root, "vanguard_Mesh")->children()[0]);
    auto animatedMesh1 = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(nodeHelper.find(m_sceneNode->root, "WorldWar_zombie")->children()[0]);

    if (animatedMesh) {
        auto animation = std::make_shared<Prisma::Animation>("../../../Resources/Landscape/animation.gltf", animatedMesh);
        auto animation1 = std::make_shared<Prisma::Animation>("../../../Resources/Landscape/animation1.gltf", animatedMesh1);

        animator = std::make_shared<Prisma::Animator>(animation);
        animator1 = std::make_shared<Prisma::Animator>(animation1);
        animatedMesh->animator(animator);
        animatedMesh1->animator(animator1);
    }

    Prisma::Physics::getInstance().physicsWorld()->dynamicsWorld->setGravity(btVector3(0.0,-10.0,0.0));
    
}

bool UserEngine::update()
{
    animator->updateAnimation(1.0 / fps());
    animator1->updateAnimation(1.0 / fps());
	return false;
}

void UserEngine::mouseCallback()
{

}
