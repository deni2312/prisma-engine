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

    // Define grid size
    const int gridSize = 10;
    const float spacing = 7.0f; // Adjust as needed for spacing between torus instances

    // Create a vector to hold all the torus instances
    std::vector<std::shared_ptr<Prisma::Mesh>> torusInstances;

    // Loop through grid positions
    for (int x = 0; x < gridSize; ++x) {
        for (int z = 0; z < gridSize; ++z) {
            for (int y = 0; y < gridSize; ++y) {
                // Calculate position for this torus instance
                glm::vec3 position(x * spacing, y * spacing, z * spacing); // Adjust y position if needed

                // Instantiate torus and apply transformation matrix
                std::shared_ptr<Prisma::Mesh> torusInstance = Prisma::Mesh::instantiate(std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(m_sceneNode->root, "Torus")->children()[0]));
                torusInstance->computeAABB();
                auto physicsComponent = std::make_shared<Prisma::PhysicsMeshComponent>();
                physicsComponent->collisionData({ Prisma::Physics::Collider::BOX_COLLIDER,1.0,btVector3(0.0,0.0,0.0),true });
                torusInstance->addComponent(physicsComponent);
                // Apply transformation matrix
                glm::mat4 matrix = glm::translate(glm::mat4(1.0f), position); // Identity matrix for now

                torusInstance->parent()->matrix(matrix);

                // Add torus instance to the vector
                torusInstances.push_back(torusInstance);
            }
        }
    }

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
    
	return false;
}

void UserEngine::mouseCallback()
{

}
