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
    m_root = Prisma::Engine::getInstance().getScene("../../../Resources/DefaultScene/default.gltf", { true });
    Prisma::Texture texture;
    texture.loadEquirectangular("../../../Resources/Skybox/equirectangular.hdr");
    texture.data({ 4096,4096,3 });
    Prisma::PipelineSkybox::getInstance().texture(texture, true);

    Prisma::NodeHelper nodeHelper;

    auto animatedMesh = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(nodeHelper.find(m_root->root, "MutantMesh")->children()[0]);

    if (animatedMesh) {
        auto animation = std::make_shared<Prisma::Animation>("../../../Resources/DefaultScene/animations/animation.gltf", animatedMesh);

        auto animator = std::make_shared<Prisma::Animator>(animation);
        animatedMesh->animator(animator);
    }

    nodeHelper.nodeIterator(m_root->root, [](auto mesh, auto parent) {
        auto currentMesh = std::dynamic_pointer_cast<Prisma::Mesh>(mesh);
        if (currentMesh) {
            auto physicsComponent = std::make_shared<Prisma::PhysicsMeshComponent>();
            physicsComponent->collisionData({ Prisma::Physics::Collider::BOX_COLLIDER,0.0,btVector3(0.0,0.0,0.0),true });
            currentMesh->addComponent(physicsComponent);
        }
    });

    Prisma::Physics::getInstance().physicsWorld()->dynamicsWorld->setGravity(btVector3(0.0, -10.0, 0.0));

    m_player = std::make_shared<PlayerController>();

    m_player->scene(m_root);
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
