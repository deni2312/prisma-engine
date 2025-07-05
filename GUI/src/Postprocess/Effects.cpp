#include "../../include/Postprocess/Effects.h"
#include "../../include/TextureInfo.h"

Prisma::GUI::Effects::Effects() {
    m_bloomRender = std::make_unique<Bloom>();
    m_volumetricRender = std::make_shared<VolumetricLight>();
    m_stylesRender = std::make_unique<PostprocessingStyles>();
    m_raysRender = std::make_shared<VolumetricRays>();
}

void Prisma::GUI::Effects::effect(PostprocessingStyles::EFFECTS effect) {
    m_effects = effect;
}

void Prisma::GUI::Effects::render() {
    m_stylesRender->render(m_effects);
    if (m_effects == PostprocessingStyles::EFFECTS::BLOOM) {
        m_bloomRender->render();
    }

    if (m_effects == PostprocessingStyles::EFFECTS::VOLUMETRIC) {
        m_volumetricRender->render();
    }

    if (m_effects == PostprocessingStyles::EFFECTS::RAYS) {
        m_raysRender->render();
    }
}

std::shared_ptr<Prisma::GUI::VolumetricRays> Prisma::GUI::Effects::volumetricRaysRender() { return m_raysRender; }

std::shared_ptr<Prisma::GUI::VolumetricLight> Prisma::GUI::Effects::volumetricRender() { return m_volumetricRender; }