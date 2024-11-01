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

	auto animatedMesh1 = Prisma::AnimatedMesh::instantiate(animatedMesh);

	if (animatedMesh)
	{
		m_walkAnimation = std::make_shared<Prisma::Animation>(
			"../../../Resources/DefaultScene/animations/idle.gltf", animatedMesh);
		auto animator = std::make_shared<Prisma::Animator>(m_walkAnimation);
		animatedMesh->animator(animator);

		auto walkAnimation1 = std::make_shared<Prisma::Animation>(
			m_walkAnimation, animatedMesh1);
		auto animator1 = std::make_shared<Prisma::Animator>(walkAnimation1);
		animatedMesh1->animator(animator1);
	}
}

std::shared_ptr<Prisma::CallbackHandler> PlayerController::callback()
{
	return nullptr;
}
