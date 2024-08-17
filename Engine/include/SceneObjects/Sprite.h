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
        Sprite();
        void loadSprite(const std::shared_ptr<Prisma::Texture>& texture,const  std::shared_ptr<Prisma::SSBO>& ssbo);
        void render();
    private:
        std::shared_ptr<Prisma::Texture> m_texture;
        std::shared_ptr<Prisma::SSBO> m_ssbo;
        unsigned int m_spritePos;
        unsigned int m_modelPos;

    };
}
