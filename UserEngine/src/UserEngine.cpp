#include "../include/UserEngine.h"
#include "../../Engine/include/Pipelines/PipelineSkybox.h"
#include "../../Engine/include/Components/PhysicsMeshComponent.h"


#include <memory>
#include <string>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#include <random>
#include <glm/gtc/random.hpp>

UserEngine::UserEngine() : Prisma::Engine{}
{
	m_sceneNode = getScene("../../../Resources/Sponza/Sponza.gltf", {true});

	Prisma::Texture texture;
	texture.loadEquirectangular("../../../Resources/Skybox/equirectangular.hdr");
	texture.data({ 4096,4096,3 });
	Prisma::PipelineSkybox::getInstance().texture(texture,true);

    Prisma::NodeHelper nodeHelper;

    nodeHelper.nodeIterator(m_sceneNode->root,[](auto mesh,auto parent){
        auto currentMesh=std::dynamic_pointer_cast<Prisma::Mesh>(mesh);
        if(currentMesh){
            auto physicsComponent = std::make_shared<Prisma::PhysicsMeshComponent>();
            physicsComponent->collisionData({Prisma::Physics::Collider::BOX_COLLIDER,0.0,btVector3(0.0,0.0,0.0),true});
            currentMesh->addComponent(physicsComponent);
        }
    });

    Prisma::Physics::getInstance().physicsWorld()->dynamicsWorld->setGravity(btVector3(0.0,-10.0,0.0));

    /*auto light = std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightOmni>>(nodeHelper.find(m_sceneNode->root, "Point")->children()[0]);

    if (light) {
        std::random_device rd; // Obtain a random number from hardware
        std::mt19937 gen(rd()); // Seed the generator
        std::uniform_real_distribution<float> disColor(0.0f, 1.0f); // Random color range

        for (int i = 0; i < 1022; i++) {
            auto lightCopy = std::make_shared<Prisma::Light<Prisma::LightType::LightOmni>>();

            // Random color
            glm::vec3 randomColor(disColor(gen), 0, disColor(gen));

            // Modify the light type properties
            auto type = light->type();
            type.radius = 1.5f;
            type.position = glm::vec4(0,i*0.1,0, 1.0f);
            type.diffuse = glm::vec4(randomColor, 1.0f);
            lightCopy->type(type);
            lightCopy->name("Light" + std::to_string(i));
            m_lights.push_back(lightCopy);
            m_sceneNode->root->addChild(lightCopy);
        }
    }*/
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

    return false;
}

void UserEngine::mouseCallback()
{

}
