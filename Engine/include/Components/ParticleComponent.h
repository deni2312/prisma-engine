#pragma once
#include <Common/interface/RefCntAutoPtr.hpp>
#include <memory>

#include "Components/RenderComponent.h"
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Helpers/PrismaRender.h"
#include "SceneObjects/Sprite.h"

namespace Prisma {
class ParticleComponent : public Component {
   public:
    ParticleComponent();

    void ui() override;

    void start() override;

    void destroy() override;

   private:
    std::shared_ptr<Prisma::Sprite> m_sprite;
    std::function<void()> m_updateUI;
    std::function<void()> m_loadTexture;
    std::function<void()> m_apply;
    unsigned int m_width=1;
    unsigned int m_height=1;
    unsigned int m_speed=1;
    glm::vec2 m_size = glm::vec2(0.1, 0.1);
};
}  // namespace Prisma