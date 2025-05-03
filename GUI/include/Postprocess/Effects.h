#pragma once
#include "Postprocess/PostprocessEffect.h"

namespace Prisma::GUI {
class Effects : public PostprocessEffect {
public:
    enum class EFFECTS {
        NORMAL,
        SEPPIA,
        CARTOON,
        VIGNETTE,
        VOLUMETRIC,
        BLOOM
    };

    Effects();

    void effect(EFFECTS effect);


    void render() override;

private:
    /* std::shared_ptr<Shader> m_shaderSeppia;
    unsigned int m_bindlessPosSeppia;

    std::shared_ptr<Shader> m_shaderCartoon;
    unsigned int m_bindlessPosCartoon;

    std::shared_ptr<Shader> m_shaderVignette;
    unsigned int m_bindlessPosVignette;

    std::shared_ptr<Shader> m_shaderBloom;
    unsigned int m_bindlessPosBloom;
    unsigned int m_horizontalPosBloom;

    std::shared_ptr<Shader> m_shaderHdr;
    unsigned int m_bindlessPosHdr;
    unsigned int m_bindlessPosHBloom;

    std::shared_ptr<FBO> m_brightnessFbo;

    std::shared_ptr<Shader> m_shaderBrightness;
    unsigned int m_bindlessPosBrightness;

    std::shared_ptr<Shader> m_shaderVolumetric;
    unsigned int m_bindlessPosVolumetric;*/

    uint64_t m_bloomTexture[2];

    unsigned int m_pingpongFBO[2];

    EFFECTS m_effects = EFFECTS::NORMAL;
};
}