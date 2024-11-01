#include "../include/PlayerController.h"
#include "../../Engine/include/Components/PhysicsMeshComponent.h"
#include "../../Engine/include/Components/CullingComponent.h"
#include <glm/gtx/string_cast.hpp>
#include "../../Engine/include/Components/CloudComponent.h"
#include "../Components/include/TerrainComponent.h"
#include "../../Engine/src/GlobalData/GlobalData.cpp"

PlayerController::PlayerController(std::shared_ptr<Prisma::Scene> scene) : m_scene{scene}
{
	Prisma::NodeHelper nodeHelper;

	auto animatedMesh = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(
		nodeHelper.find(m_scene->root, "Mesh"));
	animatedMesh->visible(false);
	int maxMeshes = 10;

	if (animatedMesh)
	{
		std::vector<std::shared_ptr<Prisma::Animation>> animations;
		animations.push_back(std::make_shared<Prisma::Animation>(
			"../../../Resources/DefaultScene/animations/walk.gltf", animatedMesh));

		animations.push_back(std::make_shared<Prisma::Animation>(
			"../../../Resources/DefaultScene/animations/jump.gltf", animatedMesh));

		animations.push_back(std::make_shared<Prisma::Animation>(
			"../../../Resources/DefaultScene/animations/death.gltf", animatedMesh));

		animations.push_back(std::make_shared<Prisma::Animation>(
			"../../../Resources/DefaultScene/animations/kick.gltf", animatedMesh));

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distr(0, animations.size() - 1);

		for (int i = 0; i < maxMeshes; i++)
		{
			for (int j = 0; j < maxMeshes; j++)
			{
				int currentAnimation = distr(gen);
				auto animatedMeshInstance = Prisma::AnimatedMesh::instantiate(animatedMesh);
				auto walkAnimationInstance = std::make_shared<Prisma::Animation>(
					animations[currentAnimation], animatedMeshInstance);
				auto animatorInstance = std::make_shared<Prisma::Animator>(walkAnimationInstance);
				animatedMeshInstance->animator(animatorInstance);
				animatedMeshInstance->parent()->parent()->matrix(
					animatedMeshInstance->parent()->parent()->matrix() * glm::translate(
						glm::mat4(1.0), glm::vec3(i * 100, j * 100, 0)));
				m_animatedMeshes.push_back(animatedMeshInstance);
			}
		}
	}
}

std::shared_ptr<Prisma::CallbackHandler> PlayerController::callback()
{
	return nullptr;
}

void PlayerController::update()
{
	for (auto mesh : m_animatedMeshes)
	{
		mesh->animator()->updateAnimation(1.0f / Prisma::Engine::getInstance().fps());
	}
}
