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
	int maxMeshes = 24;
	m_animationThread = 50;
	m_finishProgram = false;
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

		auto func = [&](int i)
		{
			while (!m_start);
			while (!m_finishProgram)
			{
				if (!m_finish[i])
				{
					int index = i * m_animationThread;
					int finish = index + m_animationThread;

					for (; index < finish && index < m_animatedMeshes.size(); index++)
					{
						m_animatedMeshes[index]->animator()->
						                         updateAnimation(1.0f / Prisma::Engine::getInstance().fps() * 2);
					}
					m_finish[i] = true;
				}
			}
		};


		for (int i = 0; i < m_animatedMeshes.size() / m_animationThread; i++)
		{
			std::thread currentThread(func, i);
			m_threads.push_back(std::move(currentThread));
			m_threads[i].detach();
			m_finish.emplace_back(false);
		}
	}
}

std::shared_ptr<Prisma::CallbackHandler> PlayerController::callback()
{
	return nullptr;
}

void PlayerController::update()
{
	m_start = true;

	for (int i = 0; i < m_finish.size(); i++)
	{
		if (m_finish[i])
		{
			int index = i * m_animationThread;
			int finish = index + m_animationThread;
			for (; index < finish && index < m_animatedMeshes.size(); index++)
			{
				m_animatedMeshes[index]->animator()->updateSSBO();
			}
			m_finish[i] = false;
		}
	}
}

void PlayerController::finish()
{
	m_finishProgram = true;
}

bool PlayerController::checkFinish()
{
	bool hasFinish = false;
	for (int i = 0; i < m_finish.size(); i++)
	{
		if (!m_finish[i])
		{
			return false;
		}
	}
	return true;
}

void PlayerController::clearFinish()
{
	for (int i = 0; i < m_finish.size(); i++)
	{
		m_finish[i] = false;
	}
}
