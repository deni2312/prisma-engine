#include "../../include/SceneObjects/Sprite.h"
#include "../../include/Helpers/IBLBuilder.h"

Prisma::Sprite::Sprite(unsigned int ssbo) {

    std::string headerVS = "#version 460 core\nlayout(std430, binding = " + std::to_string(ssbo) + ") readonly buffer SpritesData\n{\nmat4 modelSprite[];\n}; ";

    Prisma::Shader::ShaderHeaders headerData;
    headerData.vertex = headerVS;

    m_spriteShader = std::make_shared<Shader>("../../../Engine/Shaders/SpritePipeline/vertex.glsl", "../../../Engine/Shaders/SpritePipeline/fragment.glsl", nullptr, headerData);
    m_spriteShader->use();
    m_spritePos = m_spriteShader->getUniformPosition("sprite");
    m_modelPos = m_spriteShader->getUniformPosition("model");
    m_sizePos = m_spriteShader->getUniformPosition("billboardSize");
    m_ssbo = std::make_shared<Prisma::SSBO>(ssbo);
}

void Prisma::Sprite::loadSprite(const std::shared_ptr<Prisma::Texture>& texture)
{
    m_texture = texture;
}

void Prisma::Sprite::numSprites(unsigned int numSprites)
{
    m_numSprites = numSprites;
    m_ssbo->resize(sizeof(glm::mat4) * m_numSprites);
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
    m_spriteShader->use();
    m_spriteShader->setInt64(m_spritePos, m_texture->id());
    m_spriteShader->setMat4(m_modelPos, finalMatrix());
    m_spriteShader->setVec2(m_sizePos, m_size);
    Prisma::IBLBuilder::getInstance().renderQuad(m_numSprites);
}
