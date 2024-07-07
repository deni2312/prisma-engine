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
    auto root = Prisma::Engine::getInstance().getScene("../../../Resources/Landscape/landscape.prisma", { true });
    Prisma::Texture texture;
    texture.loadEquirectangular("../../../Resources/Skybox/equirectangular.hdr");
    texture.data({ 4096,4096,3 });
    Prisma::PipelineSkybox::getInstance().texture(texture, true);
    std::shared_ptr<Prisma::Animator> animator;

    Prisma::NodeHelper nodeHelper;

    nodeHelper.nodeIterator(root->root, [](auto mesh, auto parent) {
        auto currentMesh = std::dynamic_pointer_cast<Prisma::Mesh>(mesh);
        if (currentMesh) {
            auto physicsComponent = std::make_shared<Prisma::PhysicsMeshComponent>();
            physicsComponent->collisionData({ Prisma::Physics::Collider::BOX_COLLIDER,0.0,btVector3(0.0,0.0,0.0),true });
            currentMesh->addComponent(physicsComponent);
        }
        });

    auto animatedMesh = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(nodeHelper.find(root->root, "vanguard_Mesh")->children()[0]);

    if (animatedMesh) {
        auto animation = std::make_shared<Prisma::Animation>("../../../Resources/Landscape/animation.gltf", animatedMesh);

        animator = std::make_shared<Prisma::Animator>(animation);
        animatedMesh->animator(animator);
    }

    Prisma::Physics::getInstance().physicsWorld()->dynamicsWorld->setGravity(btVector3(0.0, -10.0, 0.0));
    
}

void UserEngine::update()
{

}

void UserEngine::finish()
{
}
