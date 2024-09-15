#include "../../include/SceneObjects/Sprite.h"
#include "../../include/Helpers/PrismaRender.h"
#include <glm/gtx/string_cast.hpp>

Prisma::Sprite::Sprite() {

    m_spriteShader = std::make_shared<Shader>("../../../Engine/Shaders/SpritePipeline/vertex.glsl", "../../../Engine/Shaders/SpritePipeline/fragment.glsl", nullptr);
    m_spriteShader->use();
    m_spritePos = m_spriteShader->getUniformPosition("sprite");
    m_modelPos = m_spriteShader->getUniformPosition("model");
    m_sizePos = m_spriteShader->getUniformPosition("billboardSize");
    m_ssbo = std::make_shared<Prisma::SSBO>(12);
    m_ssboTextures = std::make_shared<Prisma::SSBO>(13);
    m_ssboIds = std::make_shared<Prisma::SSBO>(14);
}

void Prisma::Sprite::loadSprites(std::vector<std::shared_ptr<Prisma::Texture>> textures)
{
    m_spritesData.clear();
    for (auto sprite : textures) {
        m_spritesData.push_back({ sprite->id(),glm::vec2(0.0) });
    }
    m_ssboTextures->resize(sizeof(SpriteData) * m_spritesData.size());
    m_ssboTextures->modifyData(0, sizeof(SpriteData) * m_spritesData.size(), m_spritesData.data());
}

void Prisma::Sprite::numSprites(unsigned int numSprites)
{
    m_numSprites = numSprites;
    m_ssbo->resize(sizeof(glm::mat4) * m_numSprites);
    m_ssboIds->resize(sizeof(glm::ivec4) * m_numSprites);
    std::vector<glm::mat4> spriteModels;
    spriteModels.resize(m_numSprites);
    glm::mat4 defaultData(1.0f);
    for (int i = 0; i < m_numSprites; i++) {
        spriteModels[i] = defaultData;
    }
    m_ssbo->modifyData(0, sizeof(glm::mat4) * m_numSprites, spriteModels.data());
}

void Prisma::Sprite::size(glm::vec2 size) {
    m_size = size;
}

void Prisma::Sprite::render()
{
    if (visible()) {
        m_spriteShader->use();
        m_spriteShader->setMat4(m_modelPos, finalMatrix());
        m_spriteShader->setVec2(m_sizePos, m_size);
        // Enable additive blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDepthMask(GL_FALSE);
        // Render the sprites
        Prisma::PrismaRender::getInstance().renderQuad(m_numSprites);
        glDepthMask(GL_TRUE);

        // Deactivate blending and restore OpenGL state
        glDisable(GL_BLEND);
    }
}
