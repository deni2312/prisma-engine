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

UserEngine::UserEngine() : Prisma::Engine{}
{
	m_sceneNode = getScene("../../../Resources/Helmet/DamagedHelmet.gltf", {true});
	Prisma::Texture texture;
	texture.loadEquirectangular("../../../Resources/Skybox/equirectangular.hdr");
	texture.data({ 4096,4096,3 });
	Prisma::PipelineSkybox::getInstance().texture(texture,true);

    Prisma::NodeHelper nodeHelper;

    auto animatedMesh = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(nodeHelper.find(m_sceneNode->root, "Vampire")->children()[0]);

    if (animatedMesh) {
        auto animation = std::make_shared<Prisma::Animation>("../../../Resources/Helmet/vampire.dae", animatedMesh);
        animator= std::make_shared<Prisma::Animator>(animation);
        animatedMesh->animator(animator);
    }

    nodeHelper.nodeIterator(m_sceneNode->root,[](auto mesh,auto parent){
        auto currentMesh=std::dynamic_pointer_cast<Prisma::Mesh>(mesh);
        if(currentMesh){
            auto physicsComponent = std::make_shared<Prisma::PhysicsMeshComponent>();
            physicsComponent->collisionData({Prisma::Physics::Collider::BOX_COLLIDER,0.0,btVector3(0.0,0.0,0.0),true});
            currentMesh->addComponent(physicsComponent);
        }
    });

    Prisma::Physics::getInstance().physicsWorld()->dynamicsWorld->setGravity(btVector3(0.0,-10.0,0.0));
    
}

bool UserEngine::update()
{
    static float time = 0.0f;
    constexpr float speed = 1.0f;  // Adjust this to change the speed of movement

    // Update time
    time += speed * 1/fps();  // Assuming deltaTime() gives the time since last frame

    // Update light positions based on sine or cosine
    for (int i = 0; i < m_lights.size(); ++i) {
        auto light = m_lights[i];

        // Move lights along Y-axis using sine or cosine functions
        float xOffset = sin(time + i * 0.5f) * 4.0f;  // Adjust amplitude and frequency
        glm::vec4 newPos = light->type().position;
        newPos.x = xOffset;

        // Update the light's position
        auto type = light->type();
        type.position = newPos;
        light->type(type);
    }
    float deltaTime = (float)1 / (float)fps();
    animator->UpdateAnimation(deltaTime);


    return false;
}

void UserEngine::mouseCallback()
{

}
