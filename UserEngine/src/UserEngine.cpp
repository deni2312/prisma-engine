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
    m_root = Prisma::Engine::getInstance().getScene("../../../Resources/Cube/cube.gltf", { true });
    Prisma::Texture texture;
    texture.loadEquirectangular("../../../Resources/Skybox/cloudy.hdr");
    texture.data({ 4096,4096,3 });
    Prisma::PipelineSkybox::getInstance().texture(texture, true);

    Prisma::NodeHelper nodeHelper;

    nodeHelper.nodeIterator(m_root->root, [](auto mesh, auto parent) {
        auto currentMesh = std::dynamic_pointer_cast<Prisma::Mesh>(mesh);
        if (currentMesh && !std::dynamic_pointer_cast<Prisma::AnimatedMesh>(mesh)) {
            auto physicsComponent = std::make_shared<Prisma::PhysicsMeshComponent>();
            physicsComponent->collisionData({ Prisma::Physics::Collider::BOX_COLLIDER,0.0,btVector3(0.0,0.0,0.0),false });
            currentMesh->addComponent(physicsComponent);
        }
    });
    
    Prisma::Physics::getInstance().physicsWorld()->dynamicsWorld->setGravity(btVector3(0.0, -10.0, 0.0));

    //m_player = std::make_shared<PlayerController>(m_root);
    //
    //m_player->scene(m_root);
}

void UserEngine::update()
{
    //m_player->update();
}

void UserEngine::finish()
{
}

std::shared_ptr<Prisma::CallbackHandler> UserEngine::callbacks()
{
    return m_player->callback();
}
