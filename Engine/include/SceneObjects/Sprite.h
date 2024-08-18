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
        void render();
    private:
        std::shared_ptr<Prisma::Texture> m_texture;
        std::shared_ptr<Prisma::SSBO> m_ssbo;
        unsigned int m_spritePos;
        unsigned int m_modelPos;
        unsigned int m_numSprites = 0;
        std::shared_ptr<Prisma::Shader> m_spriteShader;
    };
}
