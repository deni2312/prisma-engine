#include "../include/ParticleController.h"

void ParticleController::init(std::shared_ptr<Prisma::Node> root)
{

    auto spriteTexture = std::make_shared<Prisma::Texture>();
    spriteTexture->loadTexture("../../../Resources/DefaultScene/sprites/white.png");

    auto sprite = std::make_shared<Prisma::Sprite>(11);

    sprite->loadSprite(spriteTexture);
    sprite->numSprites(1000);
    sprite->size(glm::vec2(0.1f, 0.1f));
    sprite->name("Sprite");
    m_compute = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/SpriteCompute/compute.glsl");
    m_compute->use();
    m_deltaPos = m_compute->getUniformPosition("deltaTime");
    root->addChild(sprite);
}

void ParticleController::update()
{
    m_compute->use();
    m_compute->setFloat(m_deltaPos, 0.001f);
    m_compute->dispatchCompute({ 1000,1,1 });
    m_compute->wait(GL_SHADER_STORAGE_BARRIER_BIT);
}
