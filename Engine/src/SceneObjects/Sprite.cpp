#include "../../include/SceneObjects/Sprite.h"
#include "../../include/Helpers/IBLBuilder.h"

static std::shared_ptr<Prisma::Shader> spriteShader = nullptr;

Prisma::Sprite::Sprite() {

    if (!spriteShader) {
        spriteShader = std::make_shared<Shader>("../../../Engine/Shaders/SpritePipeline/vertex.glsl", "../../../Engine/Shaders/SpritePipeline/fragment.glsl");
    }
    spriteShader->use();
    m_spritePos = spriteShader->getUniformPosition("sprite");
    m_modelPos = spriteShader->getUniformPosition("model");
}

void Prisma::Sprite::loadSprite(const std::shared_ptr<Prisma::Texture>& texture, const  std::shared_ptr<Prisma::SSBO>& ssbo)
{
    m_texture = texture;
    m_ssbo = ssbo;
}

void Prisma::Sprite::render()
{
    spriteShader->use();
    spriteShader->setInt64(m_spritePos, m_texture->id());
    spriteShader->setMat4(m_modelPos, finalMatrix());
    Prisma::IBLBuilder::getInstance().renderQuad();
}
