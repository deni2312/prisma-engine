#include "../include/ParticleController.h"

void ParticleController::init(std::shared_ptr<Prisma::Node> root)
{
	auto spriteFire = std::make_shared<Prisma::Texture>();
	spriteFire->loadTexture({"../../../Resources/DefaultScene/sprites/fire.png", true});

	auto spriteBurst = std::make_shared<Prisma::Texture>();
	spriteBurst->loadTexture({"../../../Resources/DefaultScene/sprites/burst.png", true});

	auto sprite = std::make_shared<Prisma::Sprite>();

	sprite->loadSprites({spriteFire, spriteBurst});
	sprite->numSprites(1000);
	sprite->size(glm::vec2(0.1f, 0.1f));
	sprite->name("Sprite");
	m_compute = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/SpriteCompute/compute.glsl");
	m_compute->use();
	m_deltaPos = m_compute->getUniformPosition("deltaTime");
	m_timePos = m_compute->getUniformPosition("time");
	sprite->matrix(translate(glm::mat4(1.0f), glm::vec3(0, -0.5, 0)));
	root->addChild(sprite);
}

void ParticleController::update()
{
	if (!m_start)
	{
		m_startPoint = std::chrono::high_resolution_clock::now();
		m_start = true;
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - m_startPoint).count();
	m_compute->use();
	m_compute->setFloat(m_deltaPos, 1.0f / Prisma::Engine::getInstance().fps());
	m_compute->setFloat(m_timePos, static_cast<float>(duration) / 1000.0f);
	m_compute->dispatchCompute({1000, 1, 1});
	m_compute->wait(GL_SHADER_STORAGE_BARRIER_BIT);
}
