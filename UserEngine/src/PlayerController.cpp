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

		auto transparentMesh = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(m_scene->root, "Cube.012"));
		auto transparentMesh1 = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(m_scene->root, "Cube.013"));

		if (transparentMesh && transparentMesh1)
		{
			transparentMesh->material()->transparent(true);
			transparentMesh1->material()->transparent(true);
		}

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distr(0, animations.size() - 1);

		float translation = 24 * 100 / 2;

		for (int i = 0; i < maxMeshes; i++)
		{
			for (int j = 0; j < maxMeshes; j++)
			{
				int currentAnimation = distr(gen);
				auto animatedMeshInstance = Prisma::AnimatedMesh::instantiate(animatedMesh);
				auto walkAnimationInstance = std::make_shared<Prisma::Animation>(
					animations[currentAnimation], animatedMeshInstance);
				auto animatorInstance = std::make_shared<Prisma::Animator>(walkAnimationInstance);
				animatorInstance->automaticUpdate(false);
				animatedMeshInstance->animator(animatorInstance);
				animatedMeshInstance->parent()->parent()->matrix(
					animatedMeshInstance->parent()->parent()->matrix() * glm::translate(
						glm::mat4(1.0), glm::vec3(i * 100 - translation, j * 100 - translation, 0)));
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

	auto terrain = std::make_shared<Prisma::Node>();
	terrain->name("Terrain");
	terrain->matrix(translate(glm::mat4(1.0), glm::vec3(0, -4, 0)));
	auto terrainComponent = std::make_shared<Prisma::TerrainComponent>();
	auto perlin = Prisma::PrismaRender::getInstance().renderPerlin(64, 64);
	Prisma::Texture blackTexture;
	blackTexture.loadTexture({"../../../Resources/res/black.png", false, true, true, true});
	terrainComponent->heightMap(*perlin);
	terrain->addComponent(terrainComponent);
	m_scene->root->addChild(terrain);
	m_angle = 0.0;
	createLights();
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

	for (auto light : m_lights)
	{
		float angle = glm::sin(m_angle);
		auto type = light->type();
		type.position.y = angle;
		light->type(type);
	}
	m_angle = m_angle + 1.0 / Prisma::Engine::getInstance().fps();
}

void PlayerController::finish()
{
	m_finishProgram = true;
}

void PlayerController::createLights()
{
	// Set up the random number generator
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.0f, 5.0);

	float maxLights = 100;
	float translation = 5;
	for (int i = 0; i < maxLights; i++)
	{
		auto lightParent = std::make_shared<Prisma::Node>();
		auto light = std::make_shared<Prisma::Light<Prisma::LightType::LightOmni>>();
		m_lights.push_back(light);
		Prisma::LightType::LightOmni lightType;

		// Generate random translation values for x, y, and z between 0 and 100
		float x = dist(gen);
		float y = dist(gen);
		float z = dist(gen);
		lightType.diffuse = glm::vec4(x, y, z, lightType.diffuse.w);
		lightType.radius = 5;
		light->type(lightType);
		light->name("LightBall");
		lightParent->name("LightParent");
		lightParent->addChild(light);

		// Apply random translation
		lightParent->matrix(
			glm::translate(glm::mat4(1.0), glm::vec3(i / 10 - translation, 0, i % 10 - translation) * 2.0f));
		// Add the light to the scene
		m_scene->root->addChild(lightParent);
	}
}
