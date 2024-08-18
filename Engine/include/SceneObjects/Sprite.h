#pragma once
#include <vector>
#include <memory>
#include "Node.h"
#include "../Containers/Texture.h"
#include "../Helpers/Shader.h"
#include "../Containers/SSBO.h"

namespace Prisma {
    class Sprite : public Prisma::Node {
    public:
        Sprite(unsigned int ssbo);
        void loadSprite(const std::shared_ptr<Prisma::Texture>& texture);
        void numSprites(unsigned int numSprites);
        void size(glm::vec2 size);
        void render();
    private:
        std::shared_ptr<Prisma::Texture> m_texture;
        std::shared_ptr<Prisma::SSBO> m_ssbo;
        unsigned int m_spritePos;
        unsigned int m_modelPos;
        unsigned int m_numSprites = 0;
        unsigned int m_sizePos;
        glm::vec2 m_size=glm::vec2(1.0f,1.0f);
        std::shared_ptr<Prisma::Shader> m_spriteShader;
    };
}
