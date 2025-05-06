#pragma once
#include "Postprocess/PostprocessEffect.h"
#include "Bloom.h"

namespace Prisma::GUI {
class Effects : public PostprocessEffect {
public:
    enum class EFFECTS {
        NORMAL,
        SEPPIA,
        CARTOON,
        VIGNETTE,
        BLOOM
    };

    Effects();

    void effect(EFFECTS effect);


    void render() override;

private:
    std::unique_ptr<Bloom> m_bloomRender;

    EFFECTS m_effects = EFFECTS::NORMAL;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoRender;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbRender;
};
}