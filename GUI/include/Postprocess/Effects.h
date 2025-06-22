#pragma once
#include "Postprocess/PostprocessEffect.h"
#include "Bloom.h"
#include "VolumetricLight.h"
#include "VolumetricRays.h"
#include "Styles.h"

namespace Prisma::GUI {
class Effects : public PostprocessEffect {
public:
    Effects();

    void effect(PostprocessingStyles::EFFECTS effect);

    void render() override;

    std::shared_ptr<VolumetricLight> volumetricRender();
    std::shared_ptr<VolumetricRays> volumetricRaysRender();

private:
    std::unique_ptr<Bloom> m_bloomRender;
    std::shared_ptr<VolumetricLight> m_volumetricRender;
    std::unique_ptr<PostprocessingStyles> m_stylesRender;
    std::shared_ptr<VolumetricRays> m_raysRender;

    PostprocessingStyles::EFFECTS m_effects = PostprocessingStyles::EFFECTS::NORMAL;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoRender;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbRender;
};
}