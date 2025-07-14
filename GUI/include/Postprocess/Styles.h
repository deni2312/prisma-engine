#pragma once
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>

#include "Helpers/Blit.h"
#include "Helpers/TimeCounter.h"
#include "glm/glm.hpp"

namespace Prisma::GUI {
class PostprocessingStyles {
public:
    enum class EFFECTS { NORMAL, SEPPIA, CARTOON, VIGNETTE, BLOOM, VOLUMETRIC,RAYS };

    void render(EFFECTS effect);
    PostprocessingStyles();

private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_current;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_texture;
    Diligent::RefCntAutoPtr<Diligent::ITexture> m_textureCloud;

    std::unique_ptr<Blit> m_blit;

    void createShaderEffects();
    void renderEffects(EFFECTS effect);
    struct StylesData {
        glm::vec4 a;
        glm::vec2 resolution;
        glm::vec2 time;

    };

    TimeCounter m_counter;

};
}